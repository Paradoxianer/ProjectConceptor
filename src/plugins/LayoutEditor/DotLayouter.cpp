#include "DotLayouter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <OS.h>
#include <interface/Rect.h>
#include <storage/File.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <support/List.h>

#include "ProjectConceptorDefs.h"

// Graphviz's own convention (points per inch) for both its `-Tplain`
// coordinates and node width/height - not a ProjectConceptor constant.
static const float kPointsPerInch	= 72.0;


DotLayouter::DotLayouter(void)
{
}


DotLayouter::~DotLayouter(void)
{
}


bool
DotLayouter::IsAvailable(void)
{
	FILE	*pipe	= popen("dot -V 2>&1", "r");
	if (pipe == NULL)
		return false;
	char	buffer[64];
	bool	gotOutput	= (fgets(buffer,sizeof(buffer),pipe) != NULL);
	int		exitStatus	= pclose(pipe);
	return gotOutput && (exitStatus == 0);
}


status_t
DotLayouter::Layout(const BList *nodes, const BList *connections, BMessage *positions)
{
	if ((nodes == NULL) || (positions == NULL))
		return B_BAD_VALUE;
	if (nodes->CountItems() == 0)
		return B_OK;

	BPath	tempDir;
	if (find_directory(B_SYSTEM_TEMP_DIRECTORY,&tempDir) != B_OK)
		return B_ERROR;
	BString	dotPath(tempDir.Path());
	dotPath << "/projectconceptor-layout-" << find_thread(NULL) << ".dot";

	status_t	err	= WriteDotFile(dotPath,nodes,connections);
	if (err != B_OK)
		return err;

	// -Tplain has leaf node positions but no cluster geometry; -Tdot has
	// each cluster's own `bb=` but requires knowing our own subgraph
	// naming to find it (see ParseClusterBounds()) - two passes over the
	// same graph rather than one shared, more complex parser.
	BString	plainOutput;
	err	= RunDot(dotPath,"-Tplain",&plainOutput);
	if (err != B_OK) {
		remove(dotPath.String());
		return err;
	}

	BString	dotOutput;
	err	= RunDot(dotPath,"-Tdot",&dotOutput);
	remove(dotPath.String());
	if (err != B_OK)
		return err;

	float	graphHeightPoints	= 0;
	err	= ParsePlainOutput(plainOutput,nodes,positions,&graphHeightPoints);
	if (err != B_OK)
		return err;

	return ParseClusterBounds(dotOutput,nodes,graphHeightPoints,positions);
}


status_t
DotLayouter::RunDot(const BString &dotPath, const char *format, BString *output)
{
	BString	command("dot ");
	command << format << " \"" << dotPath << "\" 2>&1";
	FILE	*pipe	= popen(command.String(),"r");
	if (pipe == NULL)
		return B_ERROR;

	char	buffer[1024];
	size_t	bytesRead;
	while ((bytesRead = fread(buffer,1,sizeof(buffer)-1,pipe)) > 0) {
		buffer[bytesRead]	= '\0';
		(*output) << buffer;
	}
	int	exitStatus	= pclose(pipe);

	if (exitStatus != 0) {
		fprintf(stderr,"DotLayouter: `dot %s` exited with status %d:\n%s\n",
			format,exitStatus,output->String());
		return B_ERROR;
	}
	return B_OK;
}


status_t
DotLayouter::WriteDotFile(const BString &path, const BList *nodes, const BList *connections)
{
	BFile	file(path.String(),B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t	err	= file.InitCheck();
	if (err != B_OK)
		return err;

	BString	source("digraph G {\n");
	// only top-level nodes here (no P_C_NODE_PARENT) - a grouped node is
	// written recursively from inside its own group's WriteDotNode() call
	// instead, nested in that group's subgraph cluster block.
	for (int32 i=0; i<nodes->CountItems(); i++) {
		BMessage	*node	= (BMessage *)nodes->ItemAt(i);
		BMessage	*parent	= NULL;
		if (node->FindPointer(P_C_NODE_PARENT,(void **)&parent) != B_OK)
			WriteDotNode(source,node,nodes);
	}
	if (connections != NULL) {
		for (int32 i=0; i<connections->CountItems(); i++) {
			BMessage	*connection	= (BMessage *)connections->ItemAt(i);
			BMessage	*from		= NULL;
			BMessage	*to			= NULL;
			connection->FindPointer(P_C_NODE_CONNECTION_FROM,(void **)&from);
			connection->FindPointer(P_C_NODE_CONNECTION_TO,(void **)&to);
			int32	fromIndex	= nodes->IndexOf(from);
			int32	toIndex		= nodes->IndexOf(to);
			// skip edges to a node not in this layout run
			if ((fromIndex >= 0) && (toIndex >= 0))
				source << "  n" << fromIndex << " -> n" << toIndex << ";\n";
		}
	}
	source << "}\n";

	ssize_t	written	= file.Write(source.String(),source.Length());
	if (written < 0)
		return (status_t)written;
	if (written != source.Length())
		return B_IO_ERROR;
	return B_OK;
}


void
DotLayouter::WriteDotNode(BString &source, BMessage *node, const BList *nodes)
{
	int32	index	= nodes->IndexOf(node);
	if (index < 0)
		return;

	if (node->what == P_C_GROUP_TYPE) {
		source << "  subgraph cluster_" << index << " {\n";
		source << "    label=\"\";\n";
		// invisible anchor node - lets a connection directly to/from the
		// group itself (not one of its children) still have a real dot
		// node id to route through. The group's actual frame comes from
		// this cluster's own bb= afterward (ParseClusterBounds()), never
		// from this anchor's own position.
		source << "    n" << index << " [shape=point, style=invis, "
			"width=0.01, height=0.01];\n";
		BList	*children	= NULL;
		if ((node->FindPointer(P_C_NODE_ALLNODES,(void **)&children) == B_OK)
				&& (children != NULL)) {
			for (int32 i=0; i<children->CountItems(); i++)
				WriteDotNode(source,(BMessage *)children->ItemAt(i),nodes);
		}
		source << "  }\n";
	}
	else {
		BRect	frame(0,0,100,80);
		node->FindRect(P_C_NODE_FRAME,&frame);
		source << "  n" << index << " [width=" << (frame.Width()/kPointsPerInch)
			<< ", height=" << (frame.Height()/kPointsPerInch)
			<< ", fixedsize=true, shape=box, label=\"\"];\n";
	}
}


status_t
DotLayouter::ParsePlainOutput(const BString &output, const BList *nodes, BMessage *positions,
	float *graphHeightPoints)
{
	// "graph <scale> <width> <height>" - height needed to flip dot's
	// bottom-up coords into Haiku's top-down ones below.
	float	graphHeight	= 0;
	{
		BString	line	= output;
		int32	newline	= line.FindFirst('\n');
		if (newline >= 0)
			line.Truncate(newline);
		char	tag[16]	= {0};
		float	scale	= 0, width = 0;
		if ((sscanf(line.String(),"%15s %f %f %f",tag,&scale,&width,&graphHeight) != 4)
			|| (strcmp(tag,"graph") != 0)) {
			fprintf(stderr,"DotLayouter: unexpected `dot` output (no graph line):\n%s\n",
				output.String());
			return B_ERROR;
		}
	}
	*graphHeightPoints	= graphHeight * kPointsPerInch;

	int32	expectedLeaves	= 0;
	for (int32 i=0; i<nodes->CountItems(); i++) {
		if (((BMessage *)nodes->ItemAt(i))->what == P_C_CLASS_TYPE)
			expectedLeaves++;
	}

	int32	lineStart	= 0;
	int32	found		= 0;
	while (lineStart < output.Length()) {
		int32	lineEnd	= output.FindFirst('\n',lineStart);
		if (lineEnd < 0)
			lineEnd	= output.Length();
		BString	line;
		output.CopyInto(line,lineStart,lineEnd-lineStart);
		lineStart	= lineEnd+1;

		if (!line.StartsWith("node "))
			continue;

		char	tag[16]			= {0};
		char	name[32]		= {0};
		float	x,y,width,height;
		char	label[8]		= {0};
		// style/shape/color/fillcolor are fixed values we wrote - not
		// parsed back
		if (sscanf(line.String(),"%15s %31s %f %f %f %f %7s",
				tag,name,&x,&y,&width,&height,label) != 7)
			continue;

		int	parsedIndex	= -1;
		if ((sscanf(name,"n%d",&parsedIndex) != 1)
			|| (parsedIndex < 0) || (parsedIndex >= nodes->CountItems()))
			continue;
		int32		index	= (int32)parsedIndex;
		BMessage	*node	= (BMessage *)nodes->ItemAt(index);
		// a group's own anchor node also shows up here (it's a real,
		// if invisible, dot node) - its frame comes from the cluster's
		// own bb= instead, see ParseClusterBounds()
		if (node->what != P_C_CLASS_TYPE)
			continue;

		float	centerX	= x * kPointsPerInch;
		float	centerY	= y * kPointsPerInch;
		float	halfW	= (width * kPointsPerInch) / 2.0;
		float	halfH	= (height * kPointsPerInch) / 2.0;
		// flip dot's bottom-up y against total height for Haiku's top-down y
		BRect	frame(
			centerX - halfW,
			*graphHeightPoints - (centerY + halfH),
			centerX + halfW,
			*graphHeightPoints - (centerY - halfH));

		positions->AddPointer("node",node);
		positions->AddRect("frame",frame);
		found++;
	}

	if (found != expectedLeaves) {
		fprintf(stderr,"DotLayouter: expected %d leaf node(s) back from `dot`, got %d\n",
			(int)expectedLeaves,(int)found);
		return B_ERROR;
	}
	return B_OK;
}


status_t
DotLayouter::ParseClusterBounds(const BString &dotOutput, const BList *nodes,
	float graphHeightPoints, BMessage *positions)
{
	int32	found		= 0;
	int32	expected	= 0;
	for (int32 i=0; i<nodes->CountItems(); i++) {
		BMessage	*node	= (BMessage *)nodes->ItemAt(i);
		if (node->what != P_C_GROUP_TYPE)
			continue;
		expected++;

		// `-Tdot` writes a cluster's own "graph [bb=\"...\", ...]" line as
		// the first thing inside its block, before any nested content
		// (including a nested group's own subgraph/bb) - searching for the
		// first bb= after our marker always finds the right one.
		BString	marker;
		marker << "subgraph cluster_" << i << " ";
		int32	clusterPos	= dotOutput.FindFirst(marker.String());
		if (clusterPos < 0)
			continue;
		int32	bbPos	= dotOutput.FindFirst("bb=\"",clusterPos);
		if (bbPos < 0)
			continue;
		int32	valueStart	= bbPos+4;
		int32	quoteEnd	= dotOutput.FindFirst('"',valueStart);
		if (quoteEnd < 0)
			continue;
		BString	bbValue;
		dotOutput.CopyInto(bbValue,valueStart,quoteEnd-valueStart);

		float	llx,lly,urx,ury;
		if (sscanf(bbValue.String(),"%f,%f,%f,%f",&llx,&lly,&urx,&ury) != 4)
			continue;

		// bb is already in points (unlike -Tplain's inch-based fields) -
		// same bottom-up-to-top-down flip as ParsePlainOutput()'s leaves.
		BRect	frame(llx,graphHeightPoints-ury,urx,graphHeightPoints-lly);
		positions->AddPointer("node",node);
		positions->AddRect("frame",frame);
		found++;
	}

	if (found != expected) {
		fprintf(stderr,"DotLayouter: expected %d group bounding box(es) from `dot`, got %d\n",
			(int)expected,(int)found);
		return B_ERROR;
	}
	return B_OK;
}
