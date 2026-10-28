#include "ClassRenderer.h"
#include "ProjectConceptorDefs.h"

#include <math.h>
#include <string.h>

#include <interface/Font.h>
#include <interface/View.h>
#include <interface/GraphicsDefs.h>

#include <interface/Window.h>

#include <support/String.h>
#include "AttributRenderer.h"
#include "GroupRenderer.h"
#include "SmartGuides.h"
#include "NodeHandles.h"

// #127: candidate frames for GuidesEnabled() to align against - every
// unselected node renderer (the dragged node's own selected siblings are
// moving in lockstep this tick, see MoveAll(), and would be a moving
// target; connections have no meaningful Frame() to align to here).
static BList*
CollectGuideTargets(GraphEditor *editor)
{
	BList	*targets	= new BList();
	BList	*renderers	= editor->RenderList();
	for (int32 i=0;i<renderers->CountItems();i++) {
		Renderer	*candidate	= (Renderer *)renderers->ItemAt(i);
		if (candidate->Selected())
			continue;
		if (candidate->GetMessage()->what == P_C_CONNECTION_TYPE)
			continue;
		targets->AddItem(new BRect(candidate->Frame()));
	}
	return targets;
}

static void
DeleteGuideTargets(BList *targets)
{
	for (int32 i=0;i<targets->CountItems();i++)
		delete (BRect *)targets->ItemAt(i);
	delete targets;
}

// Screen-space snap distance, converted to document space the same way
// grid-snap's own dx/dy math already implicitly works in document units -
// GraphEditor::Scale() is applied to mouse coordinates well before they
// reach here (see GraphEditor::MouseMoved()'s scaledWhere), so "pt"/
// "startFrame" here are already document-space; the threshold just needs
// the same conversion so it stays a constant number of *screen* pixels
// regardless of zoom.
static const float	kGuideScreenThreshold	= 8.0f;


ClassRenderer::ClassRenderer(GraphEditor *parentEditor, BMessage *forContainer):Renderer(parentEditor, forContainer)
{
	TRACE();
	Init();
	ValueChanged();
	initialized	= true;
}
void ClassRenderer::Init()
{
	TRACE();
	status_t	err			 	= B_OK;
	tracking					= false;
	resizeHandle				= NodeHandles::NONE;
	columnDrag					= false;
	startColumn					= 0;
	attributeColumn				= 0;
	hovered						= false;
	doc							= NULL;
	parentNode					= NULL;

	xRadius						= 10;
	yRadius						= 10;
	attributes					= new vector<Renderer *>();
	frame						= BRect(0,0,0,0);
	selected					= false;
	font						= new AFont();
	penSize						= 1.0;
	connecting					= 0;
	hasPreviewFillColor			= false;
	hasPreviewHighlightColor	= false;
	hasPreviewBorderColor		= false;
	hasPreviewTextColor			= false;
	animating					= false;
	animPosX = animPosY		= 0;
	animVelX = animVelY		= 0;
	initialized					= false;

	BMessage	*editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);

	editMessage->AddString("Command::Name","ChangeValue");
	BMessage	*valueContainer	= new BMessage();
	valueContainer->AddString("name",P_C_NODE_NAME);
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
	valueContainer->AddInt32("type",B_STRING_TYPE);
	editMessage->AddMessage("valueContainer",valueContainer);
	name						= new StringRenderer(editor,"",BRect(0,0,100,100), editMessage);
	err 						= container->FindPointer("ProjectConceptor::doc",(void **)&doc);
	sentTo						= new BMessenger(NULL,doc);
	if (container->FindFloat(P_C_NODE_X_RADIUS,&xRadius) != B_OK)
		container->AddFloat(P_C_NODE_X_RADIUS,7.0);
	if (container->FindFloat(P_C_NODE_Y_RADIUS,&yRadius) != B_OK)
		container->AddFloat(P_C_NODE_Y_RADIUS,7.0);
	// like the radii: ChangeValue needs a field to replace
	if (container->FindFloat(P_C_NODE_ATTRIBUTE_COLUMN,&attributeColumn) != B_OK)
		container->AddFloat(P_C_NODE_ATTRIBUTE_COLUMN,0.0f);
	container->FindPointer(P_C_NODE_PARENT, (void **)&parentNode);

}


void ClassRenderer::MouseDown(BPoint where, int32 buttons,
	                              int32 clicks,int32 modifiers)
{
	// handles take precedence over the name and attribute rows below them
	int32	handle		= (selected && SupportsResize())
		? NodeHandles::HandleAt(HandleFrame(),where,HandleRadius()) : (int32)NodeHandles::NONE;
	int32	connector	= hovered ? ConnectorAt(where) : 0;
	bool	found		= (handle != NodeHandles::NONE) || (connector != 0);
	if (!found && !tracking && selected && (buttons & B_PRIMARY_MOUSE_BUTTON)
			&& ColumnBorderAt(where)) {
		if (clicks >= 2) {
			// back to fitting the names
			SendColumnWidth(0);
			return;
		}
		tracking		= true;
		columnDrag		= true;
		startMouseDown	= where;
		startColumn		= ((AttributRenderer*)(*attributes)[0])->ColumnWidth();
		editor->SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
		return;
	}
	if (!found && name->Caught(where)) {
		name->MouseDown(where);
		found	= true;
	}
	for (uint32 i = 0; (!found) && (i < attributes->size()); i++) {
		Renderer	*row	= (*attributes)[i];
		if (row->Caught(where)) {
			found	= true;
			row->MouseDown(where,buttons,clicks,modifiers);
		}
	}
	if (((handle == NodeHandles::NONE) && (connector == 0) && found) || tracking)
		return;

	if (buttons & B_PRIMARY_MOUSE_BUTTON) {
		editor->BringToFront(this);
		tracking		= true;
		startMouseDown	= where;
		oldPt			= where;
		startFrame		= frame;
		connecting		= connector;
		resizeHandle	= handle;
		editor->SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY | B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);
		if (!selected) {
			BMessage *selectMessage=new BMessage(P_C_EXECUTE_COMMAND);
			if  ((modifiers & B_SHIFT_KEY) != 0)
				selectMessage->AddBool("deselect",false);
			selectMessage->AddPointer("node",container);
			selectMessage->AddString("Command::Name","Select");
			sentTo->SendMessage(selectMessage);
		}
	}
	else if (buttons & B_SECONDARY_MOUSE_BUTTON)
		editor->SendToBack(this);
}

void ClassRenderer::MouseMoved(BPoint pt, uint32 code, const BMessage *msg) {
	if (!tracking)
		return;
	if (columnDrag) {
		attributeColumn	= startColumn + (pt.x-startMouseDown.x);
		LayoutAttributeColumn();
		editor->Invalidate();
		return;
	}
	if (connecting != 0) {
		BMessage	connecter(G_E_CONNECTING);
		connecter.AddPoint(P_C_NODE_CONNECTION_FROM,startMouseDown);
		connecter.AddPoint(P_C_NODE_CONNECTION_TO,pt);
		BMessenger((BView *)editor).SendMessage(&connecter);
		return;
	}
	if (resizeHandle != NodeHandles::NONE) {
		BRect	target	= NodeHandles::Resized(startFrame,resizeHandle,
			pt.x-startMouseDown.x,pt.y-startMouseDown.y,kMinNodeWidth,kMinNodeHeight);
		if (editor->GridEnabled())
			target	= NodeHandles::Snapped(target,resizeHandle,editor->GridWidth());
		ResizeSelected(target.left-frame.left,target.top-frame.top,
			target.right-frame.right,target.bottom-frame.bottom);
	}
	else {
		float	dx	= 0;
		float	dy	= 0;
		if (editor->GridEnabled()) {
			float newPosX = startFrame.left + (pt.x-startMouseDown.x);
			float newPosY = startFrame.top + (pt.y-startMouseDown.y);
			newPosX = newPosX - fmod(newPosX,editor->GridWidth());
			newPosY = newPosY - fmod(newPosY,editor->GridWidth());
			dx	= newPosX - frame.left;
			dy	= newPosY - frame.top;
		}
		else if (editor->GuidesEnabled()) {
			// absolute from the drag start, not incremental: guide matches
			// are recomputed against the true candidate position each tick
			float	rawDx			= pt.x - startMouseDown.x;
			float	rawDy			= pt.y - startMouseDown.y;
			BList	*targets		= CollectGuideTargets(editor);
			float	thresholdDoc	= kGuideScreenThreshold / editor->Scale();
			GuideSnapResult	snap	= ComputeGuideSnap(startFrame,rawDx,rawDy,targets,thresholdDoc);
			DeleteGuideTargets(targets);
			dx	= (startFrame.left + snap.dx) - frame.left;
			dy	= (startFrame.top + snap.dy) - frame.top;
			editor->SetActiveGuides(snap);
		}
		else {
			dx = pt.x - oldPt.x;
			dy = pt.y - oldPt.y;
		}
		BList *renderer	= editor->RenderList();
		for (int32 i=0;i<renderer->CountItems();i++)
			MoveAll(renderer->ItemAt(i),dx,dy);
	}
	oldPt	= pt;
	editor->ShowGeometry(frame);
	if (parentNode) {
		GroupRenderer	*parent	= NULL;
		if (parentNode->FindPointer(editor->RenderString(), (void **)&parent) == B_OK)
			parent->RecalcFrame();
	}
	editor->Invalidate();
}


void ClassRenderer::MouseUp(BPoint where) {
	if (columnDrag) {
		tracking	= false;
		columnDrag	= false;
		float	width	= ((AttributRenderer*)(*attributes)[0])->ColumnWidth();
		if (width != startColumn)
			SendColumnWidth(width);
		editor->Invalidate();
		return;
	}
	bool		found			= false;
	for (uint32 i = 0; (!found) && (i < attributes->size());i++) {
		Renderer	*row	= (*attributes)[i];
		if (row->Caught(where)) {
			found = true;
			row->MouseUp(where);
		}
	}
	if (found || !tracking)
		return;
	if (connecting != 0) {
		BMessage	connecter(G_E_CONNECTED);
		connecter.AddPointer(P_C_NODE_CONNECTION_FROM,container);
		connecter.AddPoint(P_C_NODE_CONNECTION_TO,where);
		BMessenger((BView *)editor).SendMessage(&connecter);
	}
	else if (resizeHandle != NodeHandles::NONE) {
		// the live preview already is the result
		BMessage	*resizer	= new BMessage(P_C_EXECUTE_COMMAND);
		resizer->AddString("Command::Name","Resize");
		resizer->AddFloat("dx",frame.right-startFrame.right);
		resizer->AddFloat("dy",frame.bottom-startFrame.bottom);
		resizer->AddFloat(P_C_RESIZE_LEFT,frame.left-startFrame.left);
		resizer->AddFloat(P_C_RESIZE_TOP,frame.top-startFrame.top);
		AdjustParents(parentNode,resizer);
		sentTo->SendMessage(resizer);
	}
	else {
		float dx = where.x - startMouseDown.x;
		float dy = where.y - startMouseDown.y;
		if (editor->GridEnabled()) {
			float newPosX = startFrame.left + dx;
			float newPosY = startFrame.top + dy;
			newPosX = newPosX - fmod(newPosX,editor->GridWidth());
			newPosY = newPosY - fmod(newPosY,editor->GridWidth());
			dx = newPosX-startFrame.left;
			dy = newPosY-startFrame.top;
		}
		else if (editor->GuidesEnabled()) {
			// matches the last on-screen preview exactly
			BList	*targets		= CollectGuideTargets(editor);
			float	thresholdDoc	= kGuideScreenThreshold / editor->Scale();
			GuideSnapResult	snap	= ComputeGuideSnap(startFrame,dx,dy,targets,thresholdDoc);
			DeleteGuideTargets(targets);
			dx	= snap.dx;
			dy	= snap.dy;
		}
		BMessage	*mover		= new BMessage(P_C_EXECUTE_COMMAND);
		mover->AddString("Command::Name","Move");
		mover->AddFloat("dx",dx);
		mover->AddFloat("dy",dy);
		AdjustParents(parentNode,mover);
		sentTo->SendMessage(mover);
	}
	tracking		= false;
	connecting		= 0;
	resizeHandle	= NodeHandles::NONE;
	editor->ClearActiveGuides();
	editor->Invalidate();
}


rgb_color ClassRenderer::TextColor(const GraphStyle &style) {
	if (hasPreviewTextColor)
		return previewTextColor;
	// like the border: the editor's standard means the style's
	rgb_color	standard;
	if ((editor->GetStandartPattern()->FindInt32("HighColor",(int32 *)&standard) == B_OK)
		&& (standard.red == textColor.red) && (standard.green == textColor.green)
		&& (standard.blue == textColor.blue))
		return style.text;
	return textColor;
}

rgb_color ClassRenderer::CardFillColor(const GraphStyle &style) {
	if (hasPreviewFillColor)
		return previewFillColor;
	// like the border: the standard fill means the style's card
	rgb_color	standard;
	if ((editor->GetStandartPattern()->FindInt32("FillColor",(int32 *)&standard) == B_OK)
		&& (standard.red == fillColor.red) && (standard.green == fillColor.green)
		&& (standard.blue == fillColor.blue))
		return style.cardFill;
	return fillColor;
}

rgb_color ClassRenderer::CardBorderColor(const GraphStyle &style) {
	if (hasPreviewBorderColor)
		return previewBorderColor;
	// the editor's standard border means "not set": then the style's,
	// which also works in a dark color scheme
	rgb_color	standard;
	if ((editor->GetStandartPattern()->FindInt32("BorderColor",(int32 *)&standard) == B_OK)
		&& (standard.red == borderColor.red) && (standard.green == borderColor.green)
		&& (standard.blue == borderColor.blue))
		return style.cardBorder;
	return borderColor;
}

void ClassRenderer::Draw(BView *drawOn, BRect updateRect) {
	// mid-slide: shift this node's whole draw (shape+name+attributes+
	// connectors, all already positioned at the real/final frame) so it
	// paints at the current animated position instead - frame itself and
	// every child stay at their real, final values throughout (see
	// AnimationStep()/ValueChanged()).
	bool	offsetForAnim	= animating;
	BPoint	priorOrigin		= drawOn->Origin();
	if (offsetForAnim) {
		BPoint	delta(animPosX-frame.left,animPosY-frame.top);
		drawOn->PushState();
		drawOn->SetOrigin(priorOrigin+delta);
	}

	const GraphStyle	&style		= editor->Style();
	const float			radius		= style.cornerRadius;
	rgb_color			bandColor	= hasPreviewHighlightColor ? previewHighlightColor : highlightColor;
	bool				fitIn		= true;
	drawOn->SetFont(font);

	rgb_color			cardColor	= CardFillColor(style);
	// a see-through card shows what lies below, not its own shadow
	bool				opaque		= (cardColor.alpha == 255);
	if (opaque)
		editor->Shadows().Draw(drawOn,shape,shape.Name(),frame,radius,
			style.shadow,style.shadowBlur,style.shadowOffsetY);

	if (selected) {
		BRect	outline	= frame;
		outline.InsetBy(-style.selectionGap,-style.selectionGap);
		NodeShape	grown(shape);
		grown.Layout(outline);
		// the gap between ring and card shows the canvas, not the shadow;
		// under a see-through card that would hide what lies below
		if (opaque) {
			drawOn->SetHighColor(style.canvas);
			if (shape.HasPath())
				grown.Fill(drawOn);
			else
				drawOn->FillRoundRect(outline,radius+style.selectionGap,radius+style.selectionGap);
		}
		drawOn->SetPenSize(style.selectionWidth);
		drawOn->SetHighColor(style.accent);
		if (shape.HasPath())
			grown.Stroke(drawOn);
		else
			drawOn->StrokeRoundRect(outline,radius+style.selectionGap,radius+style.selectionGap);
	}

	// the card, with the node's color as a band along one edge
	vector<BPoint>	outline;
	vector<BPoint>	band;
	shape.Outline(radius,&outline);
	// the border is stroked centered on the outline; half of it lies on
	// the band, so the band grows by that much to stay visible
	float	borderWidth	= BorderWidth();
	shape.AccentBand(outline,style.stripeHeight + borderWidth/2,&band);
	if (outline.size() >= 3) {
		drawOn->SetHighColor(cardColor);
		drawOn->FillPolygon(&outline[0],outline.size());
		if (band.size() >= 3) {
			drawOn->SetHighColor(bandColor);
			drawOn->FillPolygon(&band[0],band.size());
		}
		drawOn->SetPenSize(borderWidth);
		drawOn->SetHighColor(CardBorderColor(style));
		drawOn->StrokePolygon(&outline[0],outline.size(),true);
	}

	if (hovered && !selected && (outline.size() >= 3)) {
		drawOn->SetPenSize(1.5);
		drawOn->SetHighColor(GraphColors::WithAlpha(style.accent,160));
		drawOn->StrokePolygon(&outline[0],outline.size(),true);
	}

	rgb_color	text	= TextColor(style);
	rgb_color	muted	= GraphColors::Mix(text,cardColor,0.35);
	if ((text.red == style.text.red) && (text.green == style.text.green)
			&& (text.blue == style.text.blue))
		muted	= style.mutedText;
	name->SetColor(text);
	name->Draw(drawOn,updateRect);
	drawOn->SetHighColor(text);
	BRect	content	= ContentFrame();
	vector<Renderer *>::iterator	allAttributes = attributes->begin();
	while( allAttributes != attributes->end() ) {
		// #51: deleting an attribute is offered on the selected node only
		AttributRenderer	*row	= dynamic_cast<AttributRenderer*>(*allAttributes);
		if (row != NULL) {
			row->SetShowDelete(selected);
			row->SetTextColors(text,muted);
		}
		if (content.Contains((*allAttributes)->Frame()))
			(*allAttributes)->Draw(drawOn,updateRect);
		else
			fitIn=false;
		allAttributes++;
	}
	if (!fitIn) {
		drawOn->SetHighColor(style.mutedText);
		drawOn->DrawString("…",BPoint(content.left+style.paddingX,content.bottom-(style.paddingY/2)));
	}
	if (selected && SupportsResize())
		DrawHandles(drawOn,style);
	if (hovered)
		DrawConnectors(drawOn,style);

	if (offsetForAnim)
		drawOn->PopState();
}

void ClassRenderer::MessageReceived(BMessage *message) {
	switch(message->what) {
		case P_C_VALUE_CHANGED:
				ValueChanged();
			break;
	}
}


void ClassRenderer::ValueChanged() {
	TRACE();
	BMessage	*pattern		= new BMessage();
	BMessage	*messageFont	= new BMessage();
	BMessage	*data			= new BMessage();
	char		*newName		= NULL;

	char		*attribName		= NULL;
	BMessage	*attribMessage	= new BMessage();
	uint32		type			= B_ANY_TYPE;
	int32		count			= 0;

	BRect	oldFrame	= frame;
	container->FindRect(P_C_NODE_FRAME,&frame);
	if ((initialized) && (oldFrame.LeftTop() != frame.LeftTop())) {
		if (!animating) {
			animPosX	= oldFrame.left;
			animPosY	= oldFrame.top;
			animVelX	= 0;
			animVelY	= 0;
		}
		// else: already mid-animation (e.g. Auto-Layout run again before the
		// last one settled) - keep the current animated position/velocity as
		// the start of the new leg instead of snapping back to oldFrame.
		animating	= true;
		editor->StartAnimating(this);
	}
	// Get*() keeps the current value when a field is missing (a node a
	// macro inserted may have none), Find*() would zero it
	selected		= container->GetBool(P_C_NODE_SELECTED,false);
	xRadius			= container->GetFloat(P_C_NODE_X_RADIUS,xRadius);
	yRadius			= container->GetFloat(P_C_NODE_Y_RADIUS,yRadius);
	attributeColumn	= container->GetFloat(P_C_NODE_ATTRIBUTE_COLUMN,attributeColumn);
	if (container->FindMessage(P_C_NODE_PATTERN,pattern) != B_OK) {
		container->AddMessage(P_C_NODE_PATTERN,editor->GetStandartPattern());
		pattern = editor->GetStandartPattern();
	}
	if (container->FindMessage(P_C_NODE_FONT,messageFont) == B_OK) {
		delete font;
		font	= new AFont(messageFont);
	}
	container->FindMessage(P_C_NODE_DATA,data);
	pattern->FindInt32("FillColor",(int32 *)&fillColor);
	if ((container->what == P_C_CLASS_TYPE)
			&& (pattern->FindInt32("HighlightColor",(int32 *)&highlightColor) != B_OK)) {
		// before HighlightColor the fill color was the band and the card
		// was always the style's: keep that look, and give ChangeValue
		// a field to replace
		highlightColor	= fillColor;
		editor->GetStandartPattern()->FindInt32("FillColor",(int32 *)&fillColor);
		pattern->AddInt32("HighlightColor",*(int32 *)&highlightColor);
		pattern->ReplaceInt32("FillColor",*(int32 *)&fillColor);
		container->ReplaceMessage(P_C_NODE_PATTERN,pattern);
	}
	rgb_color	color;
	if (pattern->FindInt32("BorderColor",(int32 *)&color) == B_OK)
		borderColor	= color;
	if (pattern->FindInt32("HighColor",(int32 *)&textColor) != B_OK)
		editor->GetStandartPattern()->FindInt32("HighColor",(int32 *)&textColor);
	penSize	= pattern->GetFloat("PenSize",penSize);
	// a real committed value just arrived (this is only ever called from
	// a genuine P_C_VALUE_CHANGED, never from the preview path below) -
	// any leftover preview from a picker session is now stale, drop it
	// so Draw() goes back to the real fillColor just read above
	ClearPreviewColor();
	// older documents have no shape yet; like the pattern above it is
	// added so ChangeValue has a field to replace
	BMessage	shapeMessage;
	if (container->FindMessage(P_C_NODE_SHAPE,&shapeMessage) != B_OK) {
		NodeShape::BuildBuiltIn("rounded",&shapeMessage);
		container->AddMessage(P_C_NODE_SHAPE,&shapeMessage);
	}
	shape.SetTo(&shapeMessage);
	shape.Layout(frame);
	data->FindString(P_C_NODE_NAME,(const char **)&newName);
	name->SetString(newName);
	const GraphStyle	&style	= editor->Style();
	BFont	nameFont(be_bold_font);
	nameFont.SetSize(style.nameFontSize);
	name->SetFont(nameFont);
	name->SetColor(style.text);
	BRect	content		= ContentFrame();
	// a shape's text area already keeps clear of its outline
	float	padding		= shape.HasPath() ? style.paddingX/3 : style.paddingX;
	float	nameTop		= content.top+style.stripeHeight+BorderWidth()/2+(style.paddingY/2);
	name->SetFrame(BRect(content.left+padding-2,nameTop,content.right-padding,nameTop+12));
	
	
	//delete all "old" Attribs
	attributes->erase(attributes->begin(),attributes->end());
	//and add all attribs we found
	// every entry of a name gets its own row - AddAttribute appends, so
	// a name may hold several values (#138)
	for (int32 i = 0; data->GetInfo(B_MESSAGE_TYPE, i,(char **) &attribName, &type, &count) == B_OK; i++) {
		for (int32 entry = 0; entry < count; entry++) {
			if (data->FindMessage(attribName,entry,attribMessage) == B_OK)
				InsertAttribute(attribName,attribMessage, entry);
		}
	}
	container->FindPointer(P_C_NODE_PARENT, (void **)&parentNode);
	LayoutAttributeColumn();
	shape.Layout(frame);
	UpdateConnectors();

	// this node's own frame just changed via a committed command (Move,
	// ChangeValue/Auto-Layout, either one's Undo, ...) rather than an
	// interactive drag - MouseMoved()'s own live-drag path already
	// cascades to a parent group directly (same pattern as here), but
	// none of those commands mark the *parent* as changed (only this
	// node), so without this the group's box never learns a lone child
	// moved/resized outside of a drag (issue #38).
	if ((oldFrame != frame) && (parentNode != NULL)) {
		GroupRenderer	*parent	= NULL;
		if (parentNode->FindPointer(editor->RenderString(),(void **)&parent) == B_OK)
			parent->RecalcFrame();
	}
}

BRect ClassRenderer::Frame( void ) {
	return frame;
}

BRect ClassRenderer::HandleFrame(void) {
	const GraphStyle	&style	= editor->Style();
	float	out	= style.selectionGap + style.selectionWidth/2;
	return frame.InsetByCopy(-out,-out);
}


static BCursorID HandleCursor(int32 handle) {
	switch (handle) {
		case NodeHandles::TOP_LEFT:
		case NodeHandles::BOTTOM_RIGHT:
			return B_CURSOR_ID_RESIZE_NORTH_WEST_SOUTH_EAST;
		case NodeHandles::TOP_RIGHT:
		case NodeHandles::BOTTOM_LEFT:
			return B_CURSOR_ID_RESIZE_NORTH_EAST_SOUTH_WEST;
		case NodeHandles::TOP:
		case NodeHandles::BOTTOM:
			return B_CURSOR_ID_RESIZE_NORTH_SOUTH;
		default:
			return B_CURSOR_ID_RESIZE_EAST_WEST;
	}
}

BCursorID ClassRenderer::CursorAt(BPoint where) {
	// while a drag runs the cursor shows what is being dragged
	if (tracking) {
		if (columnDrag)
			return B_CURSOR_ID_RESIZE_EAST_WEST;
		if (connecting != 0)
			return B_CURSOR_ID_CROSS_HAIR;
		if (resizeHandle != NodeHandles::NONE)
			return HandleCursor(resizeHandle);
		return B_CURSOR_ID_GRABBING;
	}
	// the order MouseDown() decides by
	if (selected && SupportsResize()) {
		int32	handle	= NodeHandles::HandleAt(HandleFrame(),where,HandleRadius());
		if (handle != NodeHandles::NONE)
			return HandleCursor(handle);
	}
	if (hovered && (ConnectorAt(where) != 0))
		return B_CURSOR_ID_CROSS_HAIR;
	if (selected && ColumnBorderAt(where))
		return B_CURSOR_ID_RESIZE_EAST_WEST;
	if (name->Caught(where))
		return B_CURSOR_ID_I_BEAM;
	for (uint32 i = 0; i < attributes->size(); i++) {
		Renderer	*row	= (*attributes)[i];
		if (row->Caught(where))
			return row->CursorAt(where);
	}
	// what is left of the card moves it
	return Caught(where) ? B_CURSOR_ID_GRAB : B_CURSOR_ID_SYSTEM_DEFAULT;
}

bool ClassRenderer::StartEditingAttribute(const char *key) {
	for (int32 i = (int32)attributes->size() - 1; i >= 0; i--) {
		AttributRenderer	*row	= dynamic_cast<AttributRenderer*>((*attributes)[i]);
		if ((row != NULL) && (strcmp(row->Key(),key) == 0)) {
			row->StartEditingName();
			return true;
		}
	}
	return false;
}

float ClassRenderer::HandleRadius(void) {
	return kHandleSize / editor->Scale();
}

int32 ClassRenderer::ConnectorAt(BPoint where) {
	const BRect	dots[]	= { leftConnection, topConnection, rightConnection, bottomConnection };
	float		grow	= HandleRadius() - circleSize;
	if (grow < 0)
		grow	= 0;
	for (int32 i = 0; i < 4; i++) {
		BRect	area	= dots[i];
		area.InsetBy(-grow,-grow);
		if (area.Contains(where))
			return i + 1;
	}
	return 0;
}

void ClassRenderer::DrawConnectors(BView *drawOn, const GraphStyle &style) {
	const BRect	dots[]	= { leftConnection, topConnection, rightConnection, bottomConnection };
	drawOn->SetPenSize(1.5);
	for (int32 i = 0; i < 4; i++) {
		drawOn->SetHighColor(style.cardFill);
		drawOn->FillEllipse(dots[i]);
		drawOn->SetHighColor(style.accent);
		drawOn->StrokeEllipse(dots[i]);
	}
}

void ClassRenderer::DrawHandles(BView *drawOn, const GraphStyle &style) {
	float	half	= kHandleSize / 2 / editor->Scale();
	drawOn->SetPenSize(1.0);
	for (int32 handle = 0; handle < NodeHandles::COUNT; handle++) {
		BPoint	at		= NodeHandles::Position(HandleFrame(),handle);
		BRect	square(at.x-half,at.y-half,at.x+half,at.y+half);
		drawOn->SetHighColor(style.cardFill);
		drawOn->FillRect(square);
		drawOn->SetHighColor(style.accent);
		drawOn->StrokeRect(square);
	}
}

void ClassRenderer::UpdateConnectors(void) {
	float	yMiddle	= frame.top+(frame.Height()/2);
	float	xMiddle	= frame.left+(frame.Width()/2);
	BPoint	left	= shape.Anchor(BPoint(frame.left,yMiddle));
	BPoint	top		= shape.Anchor(BPoint(xMiddle,frame.top));
	BPoint	right	= shape.Anchor(BPoint(frame.right,yMiddle));
	BPoint	bottom	= shape.Anchor(BPoint(xMiddle,frame.bottom));
	leftConnection.Set(left.x-circleSize,left.y-circleSize,left.x+circleSize,left.y+circleSize);
	topConnection.Set(top.x-circleSize,top.y-circleSize,top.x+circleSize,top.y+circleSize);
	rightConnection.Set(right.x-circleSize,right.y-circleSize,right.x+circleSize,right.y+circleSize);
	bottomConnection.Set(bottom.x-circleSize,bottom.y-circleSize,bottom.x+circleSize,bottom.y+circleSize);
}

bool  ClassRenderer::Caught(BPoint where) {
	if (shape.HasPath() ? shape.Contains(where) : frame.Contains(where))
		return true;
	if (selected && SupportsResize()
		&& (NodeHandles::HandleAt(HandleFrame(),where,HandleRadius()) != NodeHandles::NONE))
		return true;
	return hovered && (ConnectorAt(where) != 0);
}
void  ClassRenderer::SetFrame(BRect newFrame) {
	MoveBy(newFrame.left-frame.left,newFrame.top-frame.top);
	float	dWidth	= newFrame.Width()-frame.Width();
	frame.right		= newFrame.right;
	frame.bottom	= newFrame.bottom;
	name->ResizeBy(dWidth,0);
	vector<Renderer *>::iterator	allAttributes = attributes->begin();
	while( allAttributes != attributes->end() ) {
		(*allAttributes)->ResizeBy(dWidth,0);
		allAttributes++;
	}
	LayoutAttributeColumn();
	shape.Layout(frame);
	UpdateConnectors();
}

// see the identical guard in the Move command (Move.cpp): a group's own
// MoveBy() cascades into its children, so a child that is selected in its
// own right must not be moved again from here
static bool HasSelectedAncestor(BMessage *node)
{
	BMessage	*parent		= NULL;
	bool		isSelected	= false;
	while ((node != NULL)
			&& (node->FindPointer(P_C_NODE_PARENT,(void **)&parent) == B_OK)
			&& (parent != NULL)) {
		isSelected	= false;
		if ((parent->FindBool(P_C_NODE_SELECTED,&isSelected) == B_OK) && isSelected)
			return true;
		node	= parent;
		parent	= NULL;
	}
	return false;
}

bool  ClassRenderer::MoveAll(void *arg,float dx, float dy) {
	Renderer	*renderer	= (Renderer*)arg;
	if (renderer->Selected() && !HasSelectedAncestor(renderer->GetMessage()))
		renderer->MoveBy(dx,dy);
	return false;
}

void ClassRenderer::ResizeSelected(float dLeft, float dTop, float dRight,
	float dBottom) {
	BList	*renderers	= editor->RenderList();
	for (int32 i = 0; i < renderers->CountItems(); i++) {
		ClassRenderer	*node	= dynamic_cast<ClassRenderer*>((Renderer*)renderers->ItemAt(i));
		if ((node == NULL) || !node->Selected() || !node->SupportsResize())
			continue;
		BRect	resized	= node->Frame();
		resized.left	+= dLeft;
		resized.top		+= dTop;
		resized.right	+= dRight;
		resized.bottom	+= dBottom;
		if ((resized.Width() >= kMinNodeWidth) && (resized.Height() >= kMinNodeHeight))
			node->SetFrame(resized);
	}
}

void ClassRenderer::MoveBy(float dx,float dy) {
	frame.OffsetBy(dx,dy);
	shape.Layout(frame);
	UpdateConnectors();
	name->MoveBy(dx,dy);
	vector<Renderer *>::iterator	allAttributes = attributes->begin();
	while( allAttributes != attributes->end() ) {
		(*allAttributes)->MoveBy(dx,dy);
		allAttributes++;
	}
}

void ClassRenderer::ResizeBy(float dx,float dy) {
	BRect	resized	= frame;
	if (resized.Width()+dx >= kMinNodeWidth)
		resized.right	+= dx;
	if (resized.Height()+dy >= kMinNodeHeight)
		resized.bottom	+= dy;
	SetFrame(resized);
}

void ClassRenderer::SetPreviewColor(const char *field, rgb_color color) {
	if (strcmp(field,"FillColor") == 0) {
		hasPreviewFillColor		= true;
		previewFillColor		= color;
	} else if (strcmp(field,"HighlightColor") == 0) {
		hasPreviewHighlightColor	= true;
		previewHighlightColor		= color;
	} else if (strcmp(field,"BorderColor") == 0) {
		hasPreviewBorderColor	= true;
		previewBorderColor		= color;
	} else if (strcmp(field,"HighColor") == 0) {
		hasPreviewTextColor		= true;
		previewTextColor		= color;
	}
}

void ClassRenderer::ClearPreviewColor(void) {
	hasPreviewFillColor		= false;
	hasPreviewHighlightColor	= false;
	hasPreviewBorderColor	= false;
	hasPreviewTextColor		= false;
}

bool ClassRenderer::AnimationStep(float dt) {
	if (!animating)
		return false;
	// critically-damped-ish spring toward frame.LeftTop() (the already-
	// committed target) - k=stiffness, c=damping, mass=1.
	const float	k		= 180.0f;
	const float	c		= 24.0f;
	float		targetX	= frame.left;
	float		targetY	= frame.top;
	float		accelX	= k*(targetX-animPosX) - c*animVelX;
	float		accelY	= k*(targetY-animPosY) - c*animVelY;
	animVelX	+= accelX*dt;
	animVelY	+= accelY*dt;
	animPosX	+= animVelX*dt;
	animPosY	+= animVelY*dt;

	const float	epsilonPos	= 0.5f;
	const float	epsilonVel	= 2.0f;
	if ((fabs(targetX-animPosX) < epsilonPos) && (fabs(targetY-animPosY) < epsilonPos)
			&& (fabs(animVelX) < epsilonVel) && (fabs(animVelY) < epsilonVel)) {
		animating	= false;
		return false;
	}
	return true;
}

void ClassRenderer::InsertAttribute(char *attribName,BMessage *attribute,int32 count)
{
	/*switch(attribute->what)
	{
		case B_STRING_TYPE:
		{
			BMessage*	editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
			editMessage->AddPointer("node",container);
			editMessage->AddString("Command::Name","ChangeValue");
			editMessage->AddString("subgroup",P_C_NODE_DATA);
			editMessage->AddString("name",attribName);
			attribute->FindString(P_C_NODE_NAME,(const char **)&realName);
			BString		*testString 	= new BString(attribName);
			Renderer	*testRenderer	= new StringRenderer(editor,"",BRect(frame.left+2,frame.top+10,frame.right-2,frame.bottom-2), editMessage);
			attributes->insert(pair<BString *,Renderer*>(testString,testRenderer));
			break;
		}
	}*/
	BRect	attributeRect;
	if (attributes->empty())
	{
		BRect	content	= ContentFrame();
		const GraphStyle	&style	= editor->Style();
		attributeRect = BRect(content.left+style.paddingX-2,name->Frame().bottom+style.rowSpacing,
			content.right-style.paddingX+2,content.bottom);
	}
	else
	{
		Renderer* lastRenderer = (*attributes)[attributes->size()-1];
		BRect	content	= ContentFrame();
		const GraphStyle	&style	= editor->Style();
		attributeRect = BRect(content.left+style.paddingX-2,lastRenderer->Frame().bottom+style.rowSpacing/2,
			content.right-style.paddingX+2,content.bottom);
	}
	BMessage*	editMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	editMessage->AddPointer("node",container);
	editMessage->AddString("Command::Name","ChangeValue");
	BMessage*	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
	valueContainer->AddString("subgroup",attribName);
	valueContainer->AddInt32("subgroupindex",0);
	valueContainer->AddInt32("subgroupindex",count);
	editMessage->AddMessage("valueContainer",valueContainer);
	delete valueContainer;
	BMessage*	removeAttribMessage		= new BMessage(P_C_EXECUTE_COMMAND);
	removeAttribMessage->AddPointer("node",container);
	removeAttribMessage->AddString("Command::Name","RemoveAttribute");
	valueContainer	= new BMessage();
	valueContainer->AddString("subgroup",P_C_NODE_DATA);
	valueContainer->AddString("name",attribName);
	valueContainer->AddInt32("index",count);
	removeAttribMessage->AddMessage("valueContainer",valueContainer);
	AttributRenderer	*testRenderer	= new AttributRenderer(editor,attribute,attributeRect, editMessage,removeAttribMessage);
	testRenderer->SetKey(attribName);
	attributes->push_back(testRenderer);
}

void ClassRenderer::AdjustParents(BMessage* theParent, BMessage *command) {
	if (theParent) {
		BMessage *tmpParent = theParent;
		BRect		parentRect	= BRect(0,0,0,0);

		//run through all Parents until we find the "Masterparent"
		while (tmpParent) {
			tmpParent->FindRect(P_C_NODE_FRAME,&parentRect);
			GroupRenderer	*parent	= NULL;
			if (tmpParent->FindPointer(editor->RenderString(), (void **)&parent) == B_OK) {
				if (parent->Frame() != parentRect) {
					BMessage	*changeValue		= new BMessage(P_C_EXECUTE_COMMAND);
					BMessage	*valueContainer		= new BMessage();
					changeValue->AddString("Command::Name","ChangeValue");
					changeValue->AddPointer("node",tmpParent);
					valueContainer->AddInt32("type",B_RECT_TYPE);
					valueContainer->AddString("name", P_C_NODE_FRAME );
					valueContainer->AddRect("newValue", parent->Frame());
					changeValue->AddMessage("valueContainer",valueContainer);
					command->AddMessage("PCommand::subPCommand",changeValue);
				}
			}
			tmpParent->FindPointer(P_C_NODE_PARENT, (void **)&tmpParent);
		}
	}
}


float
ClassRenderer::BorderWidth() const
{
	// older documents carry a pen size of 0
	return penSize < 1.0f ? 1.0f : penSize;
}


void
ClassRenderer::LayoutAttributeColumn(void)
{
	if (attributes->empty())
		return;
	float	usable	= (*attributes)[0]->Frame().Width() - DELETER_WIDTH;
	float	width	= attributeColumn;
	if (width <= 0) {
		for (uint32 i = 0; i < attributes->size(); i++) {
			AttributRenderer	*row	= dynamic_cast<AttributRenderer*>((*attributes)[i]);
			if ((row != NULL) && (row->LabelWidth() > width))
				width	= row->LabelWidth();
		}
		if (width > usable*0.5f)
			width	= usable*0.5f;
	}
	// a stored width still leaves room for the value; names are cut short
	const float	kMinColumn	= 12.0f;
	const float	kMinValue	= 40.0f;
	if (width > usable - kMinValue)
		width	= usable - kMinValue;
	if (width < kMinColumn)
		width	= kMinColumn;
	for (uint32 i = 0; i < attributes->size(); i++) {
		AttributRenderer	*row	= dynamic_cast<AttributRenderer*>((*attributes)[i]);
		if (row != NULL)
			row->SetColumnWidth(width);
	}
	StackAttributes();
}


void
ClassRenderer::StackAttributes(void)
{
	// a note's height follows its width, the rows below move with it
	const GraphStyle	&style	= editor->Style();
	float	top	= name->Frame().bottom + style.rowSpacing;
	for (uint32 i = 0; i < attributes->size(); i++) {
		Renderer	*row	= (*attributes)[i];
		row->MoveBy(0,top - row->Frame().top);
		top	= row->Frame().bottom + style.rowSpacing/2;
	}
	// a card is as tall as its content: the stored height is the minimum.
	// Not for shapes, whose text area itself depends on the height.
	if (SupportsResize() && !shape.HasPath() && !attributes->empty()) {
		float	needed	= attributes->back()->Frame().bottom
			+ style.paddingY - frame.top;
		if (needed > frame.Height()) {
			frame.bottom	= frame.top + needed;
			shape.Layout(frame);
			UpdateConnectors();
		}
	}
}


bool
ClassRenderer::ColumnBorderAt(BPoint where)
{
	if (!ContentFrame().Contains(where))
		return false;
	for (uint32 i = 0; i < attributes->size(); i++) {
		AttributRenderer	*row	= dynamic_cast<AttributRenderer*>((*attributes)[i]);
		if ((row == NULL) || !row->ShowsName())
			continue;
		BRect	rowFrame	= row->Frame();
		float	border		= rowFrame.left + row->ColumnWidth();
		if ((where.y >= rowFrame.top) && (where.y <= rowFrame.bottom)
				&& (fabs(where.x - border) <= 3.0f / editor->Scale()))
			return true;
	}
	return false;
}


void
ClassRenderer::SendColumnWidth(float width)
{
	BMessage	change(P_C_EXECUTE_COMMAND);
	change.AddString("Command::Name","ChangeValue");
	change.AddPointer("node",container);
	BMessage	valueContainer;
	valueContainer.AddString("name",P_C_NODE_ATTRIBUTE_COLUMN);
	valueContainer.AddInt32("type",B_FLOAT_TYPE);
	valueContainer.AddFloat("newValue",width);
	change.AddMessage("valueContainer",&valueContainer);
	sentTo->SendMessage(&change);
}
