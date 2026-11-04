#ifndef SVG_WRITER_H
#define SVG_WRITER_H

#include <interface/Rect.h>
#include <support/String.h>

#include <map>

#include "GraphDocument.h"

/** Draws a document as SVG the way the graph editor shows it: cards with
 * a colored band, the name and one row per attribute, groups behind them
 * and connections between the nearest side midpoints. Colors the
 * document leaves at the editor's standard come from CSS variables
 * (Style()), so the picture follows a light or dark color scheme.
 */
class SvgWriter {
public:
							SvgWriter(const GraphDocument &document);

	/** the <svg> element; standalone adds the XML declaration and the
	 * style sheet, embedded leaves the style to the page */
			BString			Svg(bool standalone, const char *title);
	/** every node as {"id","name","attributes":[[name,value,kind]]},
	 * for a page's details panel */
			BString			NodesJson(void);
	/** the CSS variables and classes the SVG uses */
	static	BString			Style(void);

private:
			void			_Layout(void);
			void			_WriteGroup(const GraphNode &group);
			void			_WriteConnection(const GraphConnection &connection);
			void			_WriteNode(const GraphNode &node);
			BString			_Marker(const GraphConnection &connection);

	const GraphDocument		&document;
	/** where each node is drawn: cards grow with their rows, groups with
	 * their members */
	std::map<int32, BRect>	frames;
	/** arrow heads by color, "" for the standard one */
	std::map<BString, BString>	markers;
	BString					defs;
	BString					out;
};

#endif
