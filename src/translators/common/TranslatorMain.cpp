#include <TranslatorAddOn.h>
#include <View.h>
#include <Window.h>
#include <Application.h>
#include <Alert.h>
#include <Screen.h>

#include <stdio.h>

class PCTWindow :
	public BWindow
{
public:
	PCTWindow(BRect area) :	BWindow(area, "ProjectConceptorTranslator", B_TITLED_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
	{
	}
	~PCTWindow()
	{
		be_app_messenger.SendMessage(B_QUIT_REQUESTED);
	}
};

int
main()
{
	BPoint	o	= BPoint(0,0);
	BApplication app("application/x-vnd.ProjectConceptor-translator");
	BView * v = NULL;
	BRect r(0,0,200,100);
	if (MakeConfig(NULL, &v, &r))
	{
		BAlert * err = new BAlert("Error", "Error opening ConfigView for ProjectConceptor Translator", "OK");
		err->Go();
		return 1;
	}
	PCTWindow *w = new PCTWindow(r);
	v->ResizeTo(r.Width(), r.Height());
	w->AddChild(v);
	{
		BScreen scrn;
		BRect f = scrn.Frame();
		f.InsetBy(10,23);
		if (!f.Contains(o)) 
		{
			uint32 i;
			v->GetMouse(&o, &i, false);
			o.x -= r.Width()/2;
			o.y -= r.Height()/2;
			if (o.x < f.left) o.x = f.left;
			if (o.y < f.top) o.y = f.top;
			if (o.x > f.right) o.x = f.right;
			if (o.y > f.bottom) o.y = f.bottom;
		}
	}
	w->MoveTo(o);
	w->Show();
	app.Run();
	return 0;
}
