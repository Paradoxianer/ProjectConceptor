#include <TranslatorAddOn.h>
#include <TranslationKit.h>

#include <string.h>

#include "ConfigView.h"
#include "GraphDocument.h"
#include "ProjectConceptorDefs.h"
#include "SvgWriter.h"

/** Writes a document as a single HTML page anyone can open in a browser,
 * or as a bare SVG picture (#164). The page needs nothing from outside:
 * the graph is inline SVG, a little script adds zoom, search and a panel
 * with a node's attributes. Export only. */

char	translatorName[]	= "HTMLTranslator";
char	translatorInfo[]	= "Write ProjectConceptor documents as HTML pages or SVG pictures";
int32	translatorVersion	= 100;

translation_format inputFormats[] = {
	{ P_C_DOCUMENT_RAW_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, P_C_DOCUMENT_MIMETYPE, "ProjectConceptor RAW Document" },
	{ 0, 0, 0, 0, "\0", "\0" }
};

translation_format outputFormats[] = {
	{ P_C_HTML_TYPE, B_TRANSLATOR_TEXT, 0.4, 0.8, "text/html", "HTML page" },
	{ P_C_SVG_TYPE, B_TRANSLATOR_NONE, 0.4, 0.8, "image/svg+xml", "SVG image" },
	{ 0, 0, 0, 0, "\0", "\0" }
};


static const char	*kPageStyle	= R"CSS(
*{box-sizing:border-box}
html,body{margin:0;height:100%;background:var(--canvas);color:var(--text)}
body{display:flex;flex-direction:column;font-size:14px}
.pc-bar{display:flex;align-items:center;gap:12px;padding:8px 16px;
	background:var(--card);border-bottom:1px solid var(--card-border)}
.pc-bar h1{font-size:15px;font-weight:600;margin:0;flex:1;min-width:0;
	overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.pc-bar input{font:inherit;padding:4px 8px;width:14em;max-width:40vw;
	color:var(--text);background:var(--canvas);border:1px solid var(--card-border);
	border-radius:6px}
.pc-bar button{font:inherit;min-width:2em;padding:3px 8px;cursor:pointer;
	color:var(--text);background:var(--canvas);border:1px solid var(--card-border);
	border-radius:6px}
.pc-bar button:hover{border-color:var(--accent)}
#pc-stage{flex:1;position:relative;overflow:hidden}
#pc-stage>svg{position:absolute;inset:0;width:100%;height:100%;cursor:grab;
	touch-action:none;user-select:none;-webkit-user-select:none}
#pc-stage>svg.pc-dragging{cursor:grabbing}
.pc-node,.pc-group{cursor:pointer}
.pc-dim{opacity:.18}
.pc-edge.pc-hi{stroke:var(--accent);stroke-width:2.5}
.pc-selected>.pc-card,.pc-selected>.pc-group-box{stroke:var(--accent);stroke-width:2}
#pc-panel{position:absolute;top:12px;right:12px;width:320px;
	max-width:calc(100% - 24px);max-height:calc(100% - 24px);overflow:auto;
	padding:12px 16px;
	background:var(--card);border:1px solid var(--card-border);border-radius:10px;
	box-shadow:0 4px 16px rgba(0,0,0,.18)}
#pc-panel[hidden]{display:none}
#pc-panel h2{font-size:16px;margin:0 28px 12px 0;overflow-wrap:anywhere}
#pc-panel dl{margin:0;display:grid;grid-template-columns:auto 1fr;gap:6px 12px}
#pc-panel dt{color:var(--muted)}
#pc-panel dd{margin:0;overflow-wrap:anywhere}
#pc-panel dd.pc-note{white-space:pre-wrap}
#pc-panel a{color:var(--accent)}
#pc-panel .pc-close{position:absolute;top:8px;right:8px;border:0;background:none;
	color:var(--muted);font-size:20px;cursor:pointer;line-height:1}
#pc-panel .pc-empty{color:var(--muted)}
.pc-chip{display:inline-block;width:12px;height:12px;margin-right:6px;
	vertical-align:-1px;border-radius:3px;border:1px solid var(--card-border)}
@media (max-width:600px){.pc-bar{flex-wrap:wrap}
	#pc-panel{top:auto;bottom:12px;left:12px;right:12px;width:auto;max-height:50%}}
)CSS";


static const char	*kPageScript	= R"JS(
(function(){
const svg=document.querySelector('#pc-stage>svg');
const nodes=new Map(JSON.parse(document.getElementById('pc-nodes').textContent)
	.map(n=>[String(n.id),n]));
const full=svg.viewBox.baseVal;
const vb={x:full.x,y:full.y,w:full.width,h:full.height};
const all={x:full.x,y:full.y,w:full.width,h:full.height};
svg.removeAttribute('width');svg.removeAttribute('height');
const apply=()=>svg.setAttribute('viewBox',vb.x+' '+vb.y+' '+vb.w+' '+vb.h);
function fit(){
	const r=svg.getBoundingClientRect();
	if(!r.width||!r.height)return;
	const s=Math.max(all.w/r.width,all.h/r.height,0.5);
	vb.w=r.width*s;vb.h=r.height*s;
	vb.x=all.x+(all.w-vb.w)/2;vb.y=all.y+(all.h-vb.h)/2;apply();
}
function zoom(f,cx,cy){
	const r=svg.getBoundingClientRect();
	if(cx===undefined){cx=r.width/2;cy=r.height/2;}
	const px=vb.x+cx/r.width*vb.w,py=vb.y+cy/r.height*vb.h;
	vb.w/=f;vb.h/=f;
	vb.x=px-cx/r.width*vb.w;vb.y=py-cy/r.height*vb.h;apply();
}
svg.addEventListener('wheel',e=>{
	e.preventDefault();
	const r=svg.getBoundingClientRect();
	zoom(Math.exp(-e.deltaY*0.0015),e.clientX-r.left,e.clientY-r.top);
},{passive:false});
let drag=null;
svg.addEventListener('pointerdown',e=>{
	if(e.button!==0)return;
	drag={x:e.clientX,y:e.clientY,moved:false,id:e.pointerId};
});
svg.addEventListener('pointermove',e=>{
	if(!drag)return;
	const r=svg.getBoundingClientRect(),dx=e.clientX-drag.x,dy=e.clientY-drag.y;
	if(!drag.moved){
		if(Math.abs(dx)+Math.abs(dy)<4)return;
		drag.moved=true;svg.setPointerCapture(drag.id);svg.classList.add('pc-dragging');
	}
	vb.x-=dx/r.width*vb.w;vb.y-=dy/r.height*vb.h;
	drag.x=e.clientX;drag.y=e.clientY;apply();
});
function endDrag(e){
	const d=drag;drag=null;svg.classList.remove('pc-dragging');
	if(!d||d.moved||e.type!=='pointerup')return;
	if(e.target.closest('a'))return;
	const g=e.target.closest('.pc-node,.pc-group');
	if(g)show(g.dataset.id);else hide();
}
svg.addEventListener('pointerup',endDrag);
svg.addEventListener('pointercancel',endDrag);
const mark=(id,on)=>svg.querySelectorAll('.pc-edge[data-from="'+id+'"],.pc-edge[data-to="'+id+'"]')
	.forEach(l=>l.classList.toggle('pc-hi',on));
svg.querySelectorAll('.pc-node').forEach(g=>{
	g.addEventListener('pointerenter',()=>mark(g.dataset.id,true));
	g.addEventListener('pointerleave',()=>mark(g.dataset.id,false));
});
const panel=document.getElementById('pc-panel');
function show(id){
	const n=nodes.get(id);
	if(!n)return;
	panel.querySelector('h2').textContent=n.name||'—';
	const dl=panel.querySelector('dl');
	dl.replaceChildren();
	for(const [key,value,kind] of n.attributes){
		const dt=document.createElement('dt'),dd=document.createElement('dd');
		dt.textContent=key;
		if(kind==='link'&&/^(https?|mailto|ftp):/i.test(value)){
			const a=document.createElement('a');
			a.href=value;a.textContent=value;a.target='_blank';a.rel='noopener';
			dd.append(a);
		}else if(kind==='bool'){
			dd.textContent=value==='true'?'☑':'☐';
		}else if(kind==='color'){
			const chip=document.createElement('span');
			chip.className='pc-chip';chip.style.background=value;
			dd.append(chip,value);
		}else{
			dd.textContent=value;
			if(kind==='note')dd.className='pc-note';
		}
		dl.append(dt,dd);
	}
	panel.querySelector('.pc-empty').hidden=n.attributes.length>0;
	panel.hidden=false;
	svg.querySelectorAll('.pc-selected').forEach(e=>e.classList.remove('pc-selected'));
	const g=document.getElementById('n'+id);
	if(g)g.classList.add('pc-selected');
}
function hide(){
	panel.hidden=true;
	svg.querySelectorAll('.pc-selected').forEach(e=>e.classList.remove('pc-selected'));
}
panel.querySelector('.pc-close').addEventListener('click',hide);
const search=document.getElementById('pc-search');
search.addEventListener('input',()=>{
	const t=search.value.trim().toLowerCase(),hit=new Set();
	nodes.forEach((n,id)=>{
		if(!t||n.name.toLowerCase().includes(t)||n.attributes.some(a=>
			a[0].toLowerCase().includes(t)||a[1].toLowerCase().includes(t)))hit.add(id);
	});
	svg.querySelectorAll('.pc-node,.pc-group').forEach(g=>
		g.classList.toggle('pc-dim',!!t&&!hit.has(g.dataset.id)));
	svg.querySelectorAll('.pc-edge').forEach(l=>
		l.classList.toggle('pc-dim',!!t&&!(hit.has(l.dataset.from)&&hit.has(l.dataset.to))));
});
document.querySelectorAll('[data-zoom]').forEach(b=>b.addEventListener('click',()=>{
	const z=b.dataset.zoom;
	if(z==='fit')fit();else zoom(z==='in'?1.25:0.8);
}));
document.addEventListener('keydown',e=>{
	if(e.target===search){if(e.key==='Escape'){search.value='';search.dispatchEvent(new Event('input'));}return;}
	if(e.key==='+'||e.key==='=')zoom(1.25);
	else if(e.key==='-')zoom(0.8);
	else if(e.key==='0')fit();
	else if(e.key==='Escape')hide();
	else if(e.key==='/'){e.preventDefault();search.focus();}
});
window.addEventListener('resize',fit);
fit();
})();
)JS";


static BString Page(SvgWriter &writer, const char *title)
{
	BString	json	= writer.NodesJson();
	BString	page;
	page << "<!DOCTYPE html>\n<html lang=\"en\" class=\"pc-graph\">\n<head>\n"
		<< "<meta charset=\"utf-8\">\n"
		<< "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
		<< "<meta name=\"generator\" content=\"ProjectConceptor\">\n"
		<< "<meta name=\"color-scheme\" content=\"light dark\">\n"
		<< "<title>" << XmlEscaped(title) << "</title>\n"
		<< "<style>\n" << SvgWriter::Style() << kPageStyle << "</style>\n"
		<< "</head>\n<body>\n"
		<< "<header class=\"pc-bar\">\n<h1>" << XmlEscaped(title) << "</h1>\n"
		<< "<input id=\"pc-search\" type=\"search\" placeholder=\"Search\" "
			"aria-label=\"Search the nodes\">\n"
		<< "<button data-zoom=\"out\" title=\"Zoom out (-)\">−</button>"
		<< "<button data-zoom=\"fit\" title=\"Show everything (0)\">Fit</button>"
		<< "<button data-zoom=\"in\" title=\"Zoom in (+)\">+</button>\n"
		<< "</header>\n<main id=\"pc-stage\">\n" << writer.Svg(false, title)
		<< "<aside id=\"pc-panel\" hidden>\n"
		<< "<button class=\"pc-close\" title=\"Close (Esc)\">×</button>\n"
		<< "<h2></h2>\n<dl></dl>\n<p class=\"pc-empty\">No attributes</p>\n</aside>\n"
		<< "</main>\n"
		<< "<script type=\"application/json\" id=\"pc-nodes\">" << json << "</script>\n"
		<< "<script>" << kPageScript << "</script>\n"
		<< "</body>\n</html>\n";
	return page;
}


status_t Identify(BPositionIO *inSource, const translation_format *inFormat,
	BMessage *ioExtension, translator_info *outInfo, uint32 outType)
{
	// runs for every file any app identifies: export only, so nothing
	// but a document asked for as HTML or SVG is ours
	if (((outType != P_C_HTML_TYPE) && (outType != P_C_SVG_TYPE))
		|| !LooksLikeDocument(inSource))
		return B_NO_TRANSLATOR;
	outInfo->type		= P_C_DOCUMENT_RAW_TYPE;
	outInfo->group		= B_TRANSLATOR_NONE;
	outInfo->quality	= 0.4;
	outInfo->capability	= 0.8;
	strcpy(outInfo->name, "ProjectConceptor RAW Document");
	strcpy(outInfo->MIME, P_C_DOCUMENT_MIMETYPE);
	return B_OK;
}


status_t Translate(BPositionIO *inSource, const translator_info *tInfo,
	BMessage *ioExtension, uint32 outType, BPositionIO *outDestination)
{
	if ((outType != P_C_HTML_TYPE) && (outType != P_C_SVG_TYPE))
		return B_NO_TRANSLATOR;
	inSource->Seek(0, SEEK_SET);
	GraphDocument	document;
	status_t		err	= document.Read(inSource);
	if (err != B_OK)
		return err;
	const char	*title	= (ioExtension != NULL)
		? ioExtension->GetString(P_C_FORMAT_TITLE, "ProjectConceptor")
		: "ProjectConceptor";
	SvgWriter	writer(document);
	BString		text	= (outType == P_C_HTML_TYPE) ? Page(writer, title)
		: writer.Svg(true, title);
	ssize_t		written	= outDestination->Write(text.String(), text.Length());
	if (written < 0)
		return (status_t)written;
	return (written == text.Length()) ? B_OK : B_IO_ERROR;
}


status_t MakeConfig(BMessage *ioExtension, BView **outView, BRect *outExtent)
{
	*outView	= new ConfigView(translatorName);
	return B_OK;
}


status_t GetConfigMessage(BMessage *ioExtension)
{
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_HTML_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "html");
	ioExtension->AddInt32(P_C_FORMAT_TYPE, P_C_SVG_TYPE);
	ioExtension->AddString(P_C_FORMAT_EXTENSION, "svg");
	return B_OK;
}
