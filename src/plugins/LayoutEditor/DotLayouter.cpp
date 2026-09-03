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

	BString	command("dot -Tplain \"");
	command << dotPath << "\" 2>&1";
	FILE	*pipe	= popen(command.String(),"r");
	if (pipe == NULL) {
		remove(dotPath.String());
		return B_ERROR;
	}

	BString	output;
	char	buffer[1024];
	size_t	bytesRead;
	while ((bytesRead = fread(buffer,1,sizeof(buffer)-1,pipe)) > 0) {
		buffer[bytesRead]	= '\0';
		output << buffer;
	}
	int	exitStatus	= pclose(pipe);
	remove(dotPath.String());

	if (exitStatus != 0) {
		fprintf(stderr,"DotLayouter: `dot` exited with status %d:\n%s\n",
			exitStatus,output.String());
		return B_ERROR;
	}

	return ParsePlainOutput(output,nodes,positions);
}


status_t
DotLayouter::WriteDotFile(const BString &path, const BList *nodes, const BList *connections)
{
	BFile	file(path.String(),B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	status_t	err	= file.InitCheck();
	if (err != B_OK)
		return err;

	BString	source("digraph G {\n");
	for (int32 i=0; i<nodes->CountItems(); i++) {
		BMessage	*node	= (BMessage *)nodes->ItemAt(i);
		BRect		frame(0,0,100,80);
		node->FindRect(P_C_NODE_FRAME,&frame);
		source << "  n" << i << " [width=" << (frame.Width()/kPointsPerInch)
			<< ", height=" << (frame.Height()/kPointsPerInch)
			<< ", fixedsize=true, shape=box, label=\"\"];\n";
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
			// skip connections whose endpoint isn't in this layout run
			// (e.g. layouting a selection rather than the whole graph)
			// rather than emitting an edge to an undeclared DOT node
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


status_t
DotLayouter::ParsePlainOutput(const BString &output, const BList *nodes, BMessage *positions)
{
	// "graph <scale> <width> <height>" - need the overall height to flip
	// dot's bottom-up/center-origin coordinates into Haiku's top-down,
	// top-left-origin ones (see the class comment / issue #104).
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
	float	graphHeightPoints	= graphHeight * kPointsPerInch;

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
		// only the fields this needs (name/x/y/width/height) - style,
		// shape, color and fillcolor are always the fixed values this
		// class itself writes into the DOT source, so parsing them back
		// isn't needed to place the node
		if (sscanf(line.String(),"%15s %31s %f %f %f %f %7s",
				tag,name,&x,&y,&width,&height,label) != 7)
			continue;

		int	parsedIndex	= -1;
		if ((sscanf(name,"n%d",&parsedIndex) != 1)
			|| (parsedIndex < 0) || (parsedIndex >= nodes->CountItems()))
			continue;
		int32	index	= (int32)parsedIndex;

		float	centerX	= x * kPointsPerInch;
		float	centerY	= y * kPointsPerInch;
		float	halfW	= (width * kPointsPerInch) / 2.0;
		float	halfH	= (height * kPointsPerInch) / 2.0;
		// dot's y grows upward from the bottom of the drawing - flip
		// against the drawing's own total height to get Haiku's
		// top-down y, keeping this node's own height so top/bottom
		// don't come out swapped
		BRect	frame(
			centerX - halfW,
			graphHeightPoints - (centerY + halfH),
			centerX + halfW,
			graphHeightPoints - (centerY - halfH));

		positions->AddPointer("node",nodes->ItemAt(index));
		positions->AddRect("frame",frame);
		found++;
	}

	if (found != nodes->CountItems()) {
		fprintf(stderr,"DotLayouter: expected %d node(s) back from `dot`, got %d\n",
			(int)nodes->CountItems(),(int)found);
		return B_ERROR;
	}
	return B_OK;
}
