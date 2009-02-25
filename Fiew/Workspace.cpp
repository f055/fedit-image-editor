/*
Workspace.cpp
Workspace object represents the workflow contents of MDI child window. It contains
the layers, controls them and applies actions on them, also is the core of the
Fedit Document object
*/

#include "stdafx.h"
#include "Core.h"

/*
Constructor for Bitmap
*/
Workspace::Workspace(ChildCore *core,
					 Bitmap *source)
{
	this->chicore = core;

	this->initialized = false;

	this->width = (float)source->GetWidth();
	this->height = (float)source->GetHeight();
	this->sunit = sunit;

	this->res = res;
	this->runit = runit;

	this->dmode = dmode;
	this->dunit = dunit;

	this->bkgnd = bkgnd;

	this->pxwidth = (int)this->width;
	this->pxheight = (int)this->height;

	this->layers = new List<Frame>();
	this->layers->add( new Frame(this,source) );

	this->history = NULL;
	this->selection = NULL;
}
/*
Constructor for new project
*/
Workspace::Workspace(ChildCore *core,
					 float width, float height, int sunit,
					 float res, int runit,
					 int dmode, int dunit,
					 int bkgnd)
{
	this->chicore = core;

	this->initialized = false;

	this->width = width;
	this->height = height;
	this->sunit = sunit;

	this->res = res;
	this->runit = runit;

	this->dmode = dmode;
	this->dunit = dunit;

	this->bkgnd = bkgnd;

	this->pxwidth = (int)this->width;
	this->pxheight = (int)this->height;

	Bitmap *bmp = new Bitmap(this->pxwidth,this->pxheight,Core::getPixelFormat());
	if( bkgnd > 0 ){
		Graphics *g = Graphics::FromImage(bmp);
		if( bkgnd == 1 )
			g->Clear(Core::self->getToolws()->getToolwCC()->getForeColor());
		if( bkgnd == 2 )
			g->Clear(CLR_WHITE);
		delete g;
	}

	this->layers = new List<Frame>();
	this->layers->add( new Frame(this,bmp)  );

	this->history = NULL;
	this->selection = NULL;
}
/*
Nullification constructor for loading
*/
Workspace::Workspace(ChildCore *core)
{
	this->chicore = core;

	this->initialized = false;

	this->width = 0;
	this->height = 0;
	this->sunit = 0;

	this->res = 0;
	this->runit = 0;

	this->dmode = 0;
	this->dunit = 0;

	this->bkgnd = 0;

	this->pxwidth = 0;
	this->pxheight = 0;

	this->layers = NULL;
	this->history = NULL;
	this->selection = NULL;
}

Workspace::~Workspace()
{
	if( this->layers != NULL )
		delete this->layers;
	if( this->history != NULL )
		delete this->history;

	if( this->selection != NULL )
		delete this->selection;
}

void Workspace::initialize()
{
	this->selection = NULL;

	this->history = new HistoryControler(this);

	this->selectedLayers = new List<Frame>();
	this->selectedLayers->add( this->layers->getHead() );

	this->chicore->getDrawer()->reset( this->render() );

	this->initialized = true;
}
/*
Save workspace to file
*/
bool Workspace::save(WCHAR *filepath)
{
	bool sr = false;
	bool fsr = true;

	IStorage *storage = NULL;
	IStream *stream = NULL;
	Status status = Ok;

	HRESULT hr = NULL;
	CLSID encoderClsid;

	hr = CoInitialize(NULL);
	if( FAILED(hr) )
		return sr;

	FwCHAR *path = new FwCHAR(filepath);

	hr = StgCreateDocfile(
		path->toWCHAR(), 
		STGM_DIRECT_SWMR | STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_WRITE, 
		0, 
		&storage);
	if( FAILED(hr) ){
		delete path;
		return sr;
	}

	if( Core::getEncoder(ENCPNG,&encoderClsid) == FERROR ){
		storage->Commit(STGC_DEFAULT);
		storage->Release();
		delete path;
		return sr;
	}

	FwCHAR *info = NULL;
	FwCHAR *data = NULL;
	FwCHAR *count = NULL;

	Frame *frame = NULL;
	FwCHAR *framename = NULL;

	INT namecounter = 0;

	FLOAT wspcData[WSPCSIZE];
	INT frameData[FRMDSIZE];

	wspcData[WSPC_] = NULL;
	wspcData[WSPC_W] = this->width;
	wspcData[WSPC_H] = this->height;
	wspcData[WSPC_RES] = this->res;
	wspcData[WSPC_PXW] = (FLOAT)this->pxwidth;
	wspcData[WSPC_PXH] = (FLOAT)this->pxheight;
	wspcData[WSPC_SUNIT] = (FLOAT)this->sunit;
	wspcData[WSPC_RUNIT] = (FLOAT)this->runit;
	wspcData[WSPC_DMODE] = (FLOAT)this->dmode;
	wspcData[WSPC_DUNIT] = (FLOAT)this->dunit;

	if( storage->CreateStream(
		STGWSPC,
		STGM_WRITE | STGM_SHARE_EXCLUSIVE,
		NULL,NULL,
		&stream) != S_OK ){

		storage->Commit(STGC_DEFAULT);
		storage->Release();
		delete path;
		return sr;
	}
	// write Workspace info
	if( stream->Write(wspcData,WSPCSIZE * sizeof(FLOAT),NULL ) != S_OK ){
		stream->Commit(STGC_DEFAULT);
		stream->Release();
		storage->Commit(STGC_DEFAULT);
		storage->Release();

		delete path;
		return sr;
	}
	stream->Commit(STGC_DEFAULT);
	stream->Release();

	this->layers->gotoHead();
	do {
		sr = false;

		frame = this->layers->getThat();
		framename = frame->getName();

		info = new FwCHAR(STGINFO);
		data = new FwCHAR(STGDATA);
		count = new FwCHAR(namecounter);

		info->mergeWith(count);
		data->mergeWith(count);
		
		if( (hr = storage->CreateStream(
			data->toWCHAR(),
			STGM_WRITE | STGM_SHARE_EXCLUSIVE,
			NULL,NULL,
			&stream)) == S_OK ){

			// write Frame image data
			fsr = sr = frame->setStream(stream,&encoderClsid);

			stream->Commit(STGC_DEFAULT);
			stream->Release();
		}
		else {
			fsr = sr = false;
		}
		if( sr == true ){
			if( (hr = storage->CreateStream(
				info->toWCHAR(),
				STGM_WRITE | STGM_SHARE_EXCLUSIVE,
				NULL,NULL,
				&stream)) == S_OK )
			{
				sr = true;

				frameData[FRMD_TYPE] = frame->getType();
				frameData[FRMD_X] = frame->getX();
				frameData[FRMD_Y] = frame->getY();
				frameData[FRMD_OPCT] = frame->getOpacity();
				frameData[FRMD_LOCK] = (int)frame->getIsLocked();
				frameData[FRMD_VIS] = (int)frame->getIsVisible();
				frameData[FRMD_EMPT] = (int)frame->getIsEmpty();
				frameData[FRMD_NAMELEN] = (int)framename->toLength();
				frameData[FRMD_NAMEPTR] = NULL;

				LARGE_INTEGER li;
				li.QuadPart = -1;

				// write Frame info
				if( stream->Write(frameData,FRMDSIZE * sizeof(INT),NULL) != S_OK ){
					fsr = sr = false;
					stream->Commit(STGC_DEFAULT);
					stream->Release();
				}
				/*else if( stream->Seek(li,STREAM_SEEK_CUR,NULL) != S_OK ){
					fsr = sr = false;
					stream->Release();
				}*/
				else if( stream->Write(framename->toWCHAR(),(framename->toLength() + 1) * sizeof(WCHAR),NULL) != S_OK ){
					fsr = sr = false;
					stream->Commit(STGC_DEFAULT);
					stream->Release();
				}
				else {
					stream->Commit(STGC_DEFAULT);
					stream->Release();
				}
			}
		}

		namecounter ++;
		delete count;
		delete info;
		delete data;

	} while( this->layers->next() == true );

	storage->Commit(STGC_DEFAULT | STGC_CONSOLIDATE);
	storage->Release();

	delete path;
	return fsr;
}
/*
Load Workspace form file
*/
bool Workspace::load(WCHAR *filepath)
{
	bool sr = false;
	bool fsr = true;

	IStorage *storage = NULL;
	IStream *stream = NULL;
	IEnumSTATSTG *enumstore = NULL;
	Status status = Ok;

	HRESULT hr = NULL;
	STATSTG statstg;
	memset(&statstg,0,sizeof(STATSTG));

	STATSTG *statlay = (STATSTG *)malloc(2 * sizeof(STATSTG));
	memset(&statlay[0],0,sizeof(STATSTG));
	memset(&statlay[1],0,sizeof(STATSTG));

	hr = CoInitialize(NULL);
	if( FAILED(hr) ) return sr;

	FwCHAR *path = new FwCHAR(filepath);

	hr = StgOpenStorage(
		path->toWCHAR(),
		NULL,
		STGM_DIRECT_SWMR | STGM_READ | STGM_SHARE_DENY_NONE,
		NULL,
		NULL, 
		&storage);
	if( FAILED(hr) ){ delete path; return sr; }

	hr = storage->EnumElements(NULL,NULL,NULL,&enumstore);
	if( FAILED(hr) ){ storage->Commit(STGC_DEFAULT); storage->Release(); delete path; return sr; }

	ULONG read = NULL;
	FLOAT wspcData[WSPCSIZE];
	INT frameData[FRMDSIZE];

	hr = enumstore->Next(1,&statstg,NULL);
	if( FAILED(hr) ){ storage->Commit(STGC_DEFAULT); storage->Release(); delete path; return sr; }

	hr = storage->OpenStream(
		statstg.pwcsName,
		NULL,
		STGM_READ | STGM_SHARE_EXCLUSIVE,
		NULL,
		&stream);
	if( FAILED(hr) ){ storage->Commit(STGC_DEFAULT); storage->Release(); delete path; return sr; }

	// load Workspace info
	hr = stream->Read(wspcData,WSPCSIZE * sizeof(FLOAT),&read);
	if( FAILED(hr) || WSPCSIZE * sizeof(FLOAT) != read )
		{ stream->Commit(STGC_DEFAULT); stream->Release(); storage->Commit(STGC_DEFAULT); storage->Release(); delete path; return sr; }

	this->width = wspcData[WSPC_W];
	this->height = wspcData[WSPC_H];
	this->res = wspcData[WSPC_RES];
	this->pxwidth = (int)wspcData[WSPC_PXW];
	this->pxheight = (int)wspcData[WSPC_PXH];
	this->sunit = (int)wspcData[WSPC_SUNIT];
	this->runit = (int)wspcData[WSPC_RUNIT];
	this->dmode = (int)wspcData[WSPC_DMODE];
	this->dunit = (int)wspcData[WSPC_DUNIT];

	stream->Commit(STGC_DEFAULT);
	stream->Release();
	if( statstg.pwcsName != NULL )
		CoTaskMemFree(statstg.pwcsName);

	FwCHAR *info = NULL;
	FwCHAR *data = NULL;

	Frame *frame = NULL;
	WCHAR *framename = NULL;
	Image *framebitmap = NULL;
	this->layers = new List<Frame>();

	do {
		hr = enumstore->Next(1,&statlay[0],NULL);
		if( hr == S_FALSE ){ break; }
		if( statlay[0].pwcsName[0] != STGD ){ continue; }

		data = new FwCHAR(statlay[0].pwcsName);
		info = new FwCHAR(STGINFO);
		info->mergeWith(&data->toWCHAR()[1]);

		hr = storage->OpenStream(
			info->toWCHAR(),
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			NULL,
			&stream);
		if( FAILED(hr) ){ storage->Commit(STGC_DEFAULT); storage->Release(); delete path; return sr; }

		// load Frame info
		read = 0;
		hr = stream->Read(frameData,FRMDSIZE * sizeof(INT),&read);
		if( FAILED(hr) || FRMDSIZE * sizeof(INT) != read )
			{ stream->Commit(STGC_DEFAULT); stream->Release(); storage->Commit(STGC_DEFAULT); storage->Release(); delete path; return sr; }

		if( framename != NULL )
			delete [] framename;
		framename = new WCHAR[ frameData[FRMD_NAMELEN] + 1 ];

		read = 0;
		hr = stream->Read(framename,frameData[FRMD_NAMELEN] * sizeof(WCHAR),&read);
		if( FAILED(hr) || frameData[FRMD_NAMELEN] * sizeof(WCHAR) != read )
			{ stream->Commit(STGC_DEFAULT); stream->Release(); storage->Commit(STGC_DEFAULT); storage->Release(); delete path; return sr; }

		framename[ frameData[FRMD_NAMELEN] ] = '\0';

		stream->Commit(STGC_DEFAULT);
		stream->Release();

		hr = storage->OpenStream(
			data->toWCHAR(),
			NULL,
			STGM_READ | STGM_SHARE_EXCLUSIVE,
			NULL,
			&stream);
		if( FAILED(hr) )
			{ storage->Commit(STGC_DEFAULT); storage->Release(); if( framename != NULL ) delete [] framename; delete path; return sr; }

		switch(frameData[FRMD_TYPE]){
			case FRM_NULL:
				// it's Frame - load image data
				framebitmap = Image::FromStream(stream);
				if( framebitmap == NULL )
					{ stream->Commit(STGC_DEFAULT); stream->Release(); storage->Commit(STGC_DEFAULT); storage->Release(); if( framename != NULL ) delete [] framename;
						delete path; return sr; }

				frame = new Frame(this,framebitmap);
				break;
			case FRM_TEXT:
				// it's FrameText - load text data
				FrameText::TextContainer tc;

				hr = stream->Read(&tc,sizeof(FrameText::TextContainer),&read);
				if( FAILED(hr) )
					{ stream->Commit(STGC_DEFAULT); stream->Release(); storage->Commit(STGC_DEFAULT); storage->Release(); if( framename != NULL ) delete [] framename;
							delete path; return sr; }

				tc.text = new WCHAR[tc.textlen];

				hr = stream->Read(tc.text,tc.textlen * sizeof(WCHAR),&read);
				if( FAILED(hr) || tc.textlen * sizeof(WCHAR) != read )
					{ stream->Commit(STGC_DEFAULT); stream->Release(); storage->Commit(STGC_DEFAULT); storage->Release(); if( framename != NULL ) delete [] framename;
								delete path; return sr; }

				frame = new FrameText(
					this,
					new FwCHAR(tc.text),
					new FontFamily(tc.family),
					NULL,
					tc.style,
					tc.size,
					Point(tc.orgx,tc.orgy),
					Core::UINTtoRGB(tc.col),
					tc.isAA,
					frameData[FRMD_X],
					frameData[FRMD_Y]
				);
				if( tc.text != NULL )
					delete tc.text;

				break;
		}
		stream->Commit(STGC_DEFAULT);
		stream->Release();
		if( statlay[0].pwcsName != NULL )
			CoTaskMemFree(statlay[0].pwcsName);

		frame->setXY(frameData[FRMD_X],frameData[FRMD_Y],true);
		frame->setOpacity(frameData[FRMD_OPCT]);
		frame->setLock((bool)frameData[FRMD_LOCK]);
		frame->setVisibility((bool)frameData[FRMD_VIS]);
		frame->toggleEmpty((bool)frameData[FRMD_EMPT]);
		frame->setName(new FwCHAR(framename));

		this->layers->add(frame);

		delete info;
		delete data;

	} while( hr == S_OK );

	hr = storage->Commit(STGC_DEFAULT);
	hr = storage->Release();
	if( framename != NULL )
		delete [] framename;

	delete path;
	return fsr;
}
/*
Push new view of Workspace to scene Layer in Drawer
*/
void Workspace::update()
{
	this->chicore->getDrawer()->getScene()->loadContent( this->render(),UPDATE );
}
/*
Flat render of all layers of Workspace
*/
Bitmap *Workspace::render()
{
	Frame *lay = NULL;

	int w = this->getPxWidth();
	int h = this->getPxHeight();

	Bitmap *bmp = new Bitmap(w,h,Core::getPixelFormat());

	Graphics *g = Graphics::FromImage(bmp);
	g->SetInterpolationMode(InterpolationModeNearestNeighbor);

	this->layers->gotoHead();
	do {
		lay = this->layers->getThat();

		if( lay->getRender() == NULL )
			continue;

		g->DrawImage(
			lay->getRender(),
			Rect(
				lay->getX(),
				lay->getY(),
				lay->getWidth(),
				lay->getHeight() ),
			0,0,
			lay->getWidth(),
			lay->getHeight(),
			UnitPixel,
			lay->getIatt()
			);
		
	} while( this->layers->next() == true );

	delete g;
	return bmp;
}
/*
Render contents above currently selected layer
*/
Bitmap *Workspace::renderAbove(Rect clip)
{
	this->layers->gotoTail();
	if( this->layers->getThat() == this->getSelectedLayer() )
		return NULL;;

	Frame *lay = NULL;
	Bitmap *bmp = new Bitmap(clip.Width,clip.Height,Core::getPixelFormat());

	Graphics *g = Graphics::FromImage(bmp);
	g->SetInterpolationMode(InterpolationModeNearestNeighbor);

	this->layers->gotoThat( this->getSelectedLayer() );
	while( this->layers->next() == true ){
		lay = this->layers->getThat();

		if( lay->getRender() == NULL )
			continue;

		g->DrawImage(
			lay->getRender(),
			Rect(
				0,
				0,
				clip.Width,
				clip.Height ),
			clip.X - lay->getX(),
			clip.Y - lay->getY(),
			clip.Width,
			clip.Height,
			UnitPixel,
			lay->getIatt()
			);
	}
	delete g;
	return bmp;
}
/*
Render contents below currently selected layer, including that layer
*/
Bitmap *Workspace::renderBelow(Rect clip, bool withGrid)
{
	Frame *lay = NULL;
	Bitmap *bmp = new Bitmap(clip.Width,clip.Height,Core::getPixelFormat());

	Graphics *g = Graphics::FromImage(bmp);
	g->SetInterpolationMode(InterpolationModeNearestNeighbor);

	if( withGrid == true ){
		this->chicore->getDrawer()->renderBackground(g,this->chicore->getDrawer()->getZoom(),true);
	}

	this->layers->gotoHead();
	do {
		lay = this->layers->getThat();

		if( lay->getRender() == NULL )
			continue;
		if( withGrid == true && this->layers->getThat() == this->getSelectedLayer() )
			break;

		g->DrawImage(
			lay->getRender(),
			Rect(
				0,
				0,
				clip.Width,
				clip.Height ),
			clip.X - lay->getX(),
			clip.Y - lay->getY(),
			clip.Width,
			clip.Height,
			UnitPixel,
			lay->getIatt()
			);

		if( this->layers->getThat() == this->getSelectedLayer() )
			break;
	} while( this->layers->next() == true );

	delete g;
	return bmp;
}
/*
Resize Workspace size. It does not resizes the layers.
*/
void Workspace::resizeCanvas(RECT canvas, bool historical, Tool *executor)
{
	//this->width - 
	//this->height - convert real metrics

	if( historical == false ){
		HEToolCrop *he = new HEToolCrop(this,NULL,executor);

		he->set(canvas);

		this->getHistory()->add(he);
	}

	this->pxwidth = canvas.right - canvas.left;
	this->pxheight = canvas.bottom - canvas.top;

	this->layers->gotoHead();
	do {
		this->layers->getThat()->addXY(-canvas.left,-canvas.top);
	} while( this->layers->next() == true );

	this->updateToolws();
	this->update();

	this->chicore->setText();
}
/*
Resize Workspace and layers size.
*/
void Workspace::resizeImage(int width, int height, InterpolationMode mode, bool historical, Tool *executor)
{
	//this->width - 
	//this->height - convert real metrics

	double wf = (double)width / (double)this->pxwidth;
	double hf = (double)height / (double)this->pxheight;


	Rect rectBefore = Rect(0,0,this->pxwidth,this->pxheight);
	Rect rectAfter = Rect(0,0,width,height);
	List<Frame> *layersAfter = new List<Frame>();

	this->layers->gotoHead();
	do {
		layersAfter->add( this->layers->getThat()->clone() );
	} while( this->layers->next() == true );

	layersAfter->gotoHead();
	do {
		layersAfter->getThat()->resizeFrame(wf,hf,mode);
	} while( layersAfter->next() == true );

	if( historical == false ){
		HEToolResize *he = new HEToolResize(this,NULL,NULL);

		he->set(this->layers,layersAfter,rectBefore,rectAfter);

		this->getHistory()->add(he);
	}
	this->setLayers(layersAfter);

	RECT canvas;
	SetRect(&canvas,0,0,width,height);
	this->resizeCanvas(canvas,true);
}
/*
Set selecting path
*/
void Workspace::setSelection(GraphicsPath *path)
{
	if( this->selection != NULL ){
		delete this->selection;
	}
	this->selection = path;

	Rect bounds(0,0,0,0);
	if( this->selection != NULL ){
		this->selection->GetBounds(&bounds);		
	}
	Core::self->getToolws()->getToolwBoxInfo()->loadSize(bounds);
}

void Workspace::setLayers(List<Frame> *layers)
{
	int sid = this->selectedLayers->getIndex( this->getSelectedLayer() );

	this->layers = layers;
	
	this->setSelectedLayer( this->layers->setIndex(sid) );
}

void Workspace::setPxSize(int width, int height)
{
	this->pxwidth = width;
	this->pxheight = height;
}
void Workspace::setPxSize(Rect size)
{
	this->pxwidth = size.Width;
	this->pxheight = size.Height;
}

int Workspace::getPxWidth()
{
	return this->pxwidth;
}
int Workspace::getPxHeight()
{
	return this->pxheight;
}

GraphicsPath *Workspace::getSelection()
{
	return this->selection;
}

void Workspace::updateToolws()
{
	Core::self->getToolws()->getToolwBoxLayers()->load();
	Core::self->getToolws()->getToolwBoxHistory()->load();
}

void Workspace::addLayerAfter(Frame *newLayer, Frame *layer, bool historical, Tool *executor)
{
	Frame *selFrame = layer;
	if( selFrame == NULL )
		selFrame = this->getSelectedLayer();

	Frame *newFrame = newLayer;
	if( newFrame == NULL )
		newFrame = new Frame(this,1,1);

	this->layers->gotoThat(selFrame);
	this->layers->addToThat(newFrame);

	this->setSelectedLayer(newFrame);

	if( historical == false ){
		HEToolManageFrame *he = new HEToolManageFrame(this,newFrame,executor);

		he->set(selFrame,ADD);

		this->getHistory()->add(he);

		this->updateToolws();
		this->update();
	}
}

void Workspace::deleteLayer(Frame *layer, bool historical)
{
	if( this->layers->getCount() > 1 ){
		this->selectedLayers->remove(layer);

		int idx = 0;
		Frame *selee = this->selectedLayers->getThat();
		if( selee == NULL ){
			idx = this->layers->getIndex(layer) - 1;
		}
		this->layers->remove(layer);

		if( selee == NULL ){
			this->layers->setIndex(idx);
			selee = this->layers->getThat();
		}
		this->setSelectedLayer(selee);

		if( historical == false ){
			HEToolManageFrame *he = new HEToolManageFrame(this,layer,NULL);

			he->set(this->getSelectedLayer(),REMOVE);

			this->getHistory()->add(he);

			this->updateToolws();
			this->update();
		}
	}
}
/*
Move current layer up in layer order
*/
bool Workspace::moveSelectedUp(bool historical)
{
	this->layers->gotoThat( this->getSelectedLayer() );
	if( this->layers->isThatTail() == true )
		return false;

	this->layers->moveThatRight();

	HEToolManageFrameOrder *he = new HEToolManageFrameOrder(this,this->getSelectedLayer(),NULL);
	he->set(TOP);

	this->getHistory()->add(he);

	this->update();
	return true;
}
/*
Move current layer down in layer order
*/
bool Workspace::moveSelectedDown(bool historical)
{
	this->layers->gotoThat( this->getSelectedLayer() );
	if( this->layers->isThatHead() == true )
		return false;

	this->layers->moveThatLeft();

	HEToolManageFrameOrder *he = new HEToolManageFrameOrder(this,this->getSelectedLayer(),NULL);
	he->set(BOT);

	this->getHistory()->add(he);

	this->update();
	return true;
}
/*
Merge current layer with the one below it
*/
void Workspace::mergeDownSelected()
{
	if( this->isMergeReady() == true ){
		int idx = this->layers->getIndex(this->getSelectedLayer()) - 1;
		Frame *before = this->layers->setIndex(idx);

		Bitmap *render = this->getSelectedLayer()->getRender();

		Rect trgtRect(
			this->getSelectedLayer()->getX(),
			this->getSelectedLayer()->getY(),
			this->getSelectedLayer()->getWidth(),
			this->getSelectedLayer()->getHeight() );
		Rect srcRect(0,0,trgtRect.Width,trgtRect.Height);

		this->deleteLayer(this->getSelectedLayer());
		before->applyPaint(render,trgtRect,srcRect,Core::self->getToolws()->getToolwCC()->toolMerge);
	}
}
bool Workspace::isMergeReady()
{
	int idx = this->layers->getIndex(this->getSelectedLayer()) - 1;
	Frame *before = this->layers->setIndex(idx);

	if( this->layers->getHead() != this->getSelectedLayer() &&
		this->getSelectedLayer()->isFrameReady() == true &&
		before->getType() != FRM_TEXT )
		return true;

	return false;
}
/*
Rasterize text layer
*/
void Workspace::rasterizeSelected()
{
	if( this->isRasterizeReady() == true ){		
		Bitmap *render = this->getSelectedLayer()->getRender();

		Rect trgtRect(
			this->getSelectedLayer()->getX(),
			this->getSelectedLayer()->getY(),
			this->getSelectedLayer()->getWidth(),
			this->getSelectedLayer()->getHeight() );
		Rect srcRect(0,0,trgtRect.Width,trgtRect.Height);

		this->layers->gotoThat(this->getSelectedLayer());

		Frame *after = new Frame(this,render,trgtRect.X,trgtRect.Y);
		Frame *before = this->layers->swapThat(after);
		this->setSelectedLayer(after);

		HEToolRaster *he = new HEToolRaster(this,after,Core::self->getToolws()->getToolwCC()->toolRaster);

		he->set(before,after);

		this->getHistory()->add(he);

		this->updateToolws();
		this->update();
	}
}
bool Workspace::isRasterizeReady()
{
	if( this->getSelectedLayer()->getType() == FRM_TEXT &&
		this->getSelectedLayer()->isFrameReady() == true )
		return true;

	return false;
}

List<Frame> *Workspace::getLayers()
{
	return this->layers;
}

List<Frame> *Workspace::getSelectedLayers()
{
	return this->selectedLayers;
}

Frame *Workspace::getSelectedLayer()
{
	return this->selectedLayers->getTail();
}

int Workspace::getSelectedLayerIndex()
{
	int idx = 0;

	this->layers->gotoTail();
	do {
		if( this->layers->getThat() == this->getSelectedLayer() )
			return idx;
		idx++;
	} while( this->layers->prev() == true );

	return idx;
}
/*
Get current layer bitmap projection onto view 
of workspace size or layer size (inflate flag)
*/
Bitmap *Workspace::getSelectedLayerProjection(bool inflate)
{
	Frame *frame = this->getSelectedLayer();
	Bitmap *render = frame->getRender();
	Bitmap *proj = NULL;
	Graphics *gp = NULL;

	Rect ur(0,0,this->pxwidth,this->pxheight);

	if( inflate == true ){
		Rect wr = Rect(0,0,this->pxwidth,this->pxheight);
		Rect ir = Rect(frame->getX(),frame->getY(),frame->getHeight(),frame->getWidth());

		Rect::Union(ur,wr,ir);
	}
	if( ur.Width > this->pxwidth || ur.Height > this->pxheight ){
		proj = new Bitmap(ur.Width,ur.Height,render->GetPixelFormat());
		gp = Graphics::FromImage(proj);
		gp->DrawImage(
			render,
			Rect(max(frame->getX(),0),max(frame->getY(),0),frame->getWidth(),frame->getHeight()),
			0,0,frame->getWidth(),frame->getHeight(),
			UnitPixel
			);
		delete gp;
	}
	else {
		proj = new Bitmap(this->pxwidth,this->pxheight,render->GetPixelFormat());
		gp = Graphics::FromImage(proj);
		gp->DrawImage(
			render,
			Rect(this->getSelectedLayer()->getX(),
				 this->getSelectedLayer()->getY(),
				 render->GetWidth(),
				 render->GetHeight() ),
			0,0,
			render->GetWidth(),
			render->GetHeight(),
			UnitPixel
			);
		delete gp;
	}

	return proj;
}
/*
Paste bitmap info either on the center of current view
or the current seleciton
*/
void Workspace::setSelectedProjection(Bitmap *bmp, int mode)
{
	int x = 0;
	int y = 0;

	if( this->selection != NULL ){
		Rect bounds;
		this->selection->GetBounds(&bounds);

		int offx = (bounds.Width - (int)bmp->GetWidth()) / 2;
		int offy = (bounds.Height - (int)bmp->GetHeight()) / 2;

		x = bounds.X + offx;
		y = bounds.Y + offy;
	}
	else {
		Point center = this->chicore->getDrawer()->getCenter();
		x = (int)( center.X - floor((double)bmp->GetWidth() / 2.0) );
		y = (int)( center.Y - floor((double)bmp->GetHeight() / 2.0) );
	}

	if( this->getSelectedLayer()->getIsEmpty() == true ){
		this->getSelectedLayer()->applyPaint(
			bmp,
			Rect(x,y,bmp->GetWidth(),bmp->GetHeight()),
			Rect(0,0,bmp->GetWidth(),bmp->GetHeight()),
			Core::self->getToolws()->getToolwCC()->toolPaste
			);
	}
	else {
		Frame *frame = new Frame(this,bmp,x,y);
		this->addLayerAfter(
			frame,
			NULL,
			false,
			Core::self->getToolws()->getToolwCC()->toolPaste
			);
	}
}
/*
Copy bitmap info of whole Workspace basing on the given rectangle
*/
Bitmap *Workspace::getSelectedProjection(Rect *rect, int mode)
{
	if( this->selection != NULL ){
		Rect bounds;
		Bitmap *outBmp = NULL;
		Graphics *outGfx = NULL;

		Frame *frame = this->getSelectedLayer();
		GraphicsPath *selee = this->selection->Clone();
		GraphicsPath *selclr = this->selection->Clone();

		selee->GetBounds(&bounds);

		Bitmap *render = NULL;

		if( mode == COPCPM )
			render = this->render();
		else 
			render = frame->getRender();

		Matrix mx, my;
		mx.Translate((REAL)-frame->getX(),(REAL)-frame->getY());
		selclr->Transform(&mx);

		switch(mode){
			case COPCLR:
				frame->applyClear(selclr,Core::self->getToolws()->getToolwCC()->toolClear);
				this->update();
				break;
			case COPCOP:
			case COPCUT:
				outBmp = new Bitmap(bounds.Width-1,bounds.Height-1,render->GetPixelFormat());
				outGfx = Graphics::FromImage(outBmp);

				my.Translate((REAL)-bounds.X,(REAL)-bounds.Y);
				selee->Transform(&my);

				outGfx->SetClip(selee);
				outGfx->DrawImage(
					render,
					Rect(0,0,outBmp->GetWidth(),outBmp->GetHeight()),
					bounds.X - frame->getX(),
					bounds.Y - frame->getY(),
					outBmp->GetWidth(),
					outBmp->GetHeight(),
					UnitPixel
					);

				if( mode == COPCUT ){
					frame->applyClear(selclr,Core::self->getToolws()->getToolwCC()->toolCut);
					this->update();
				}
				break;
			case COPCPM:
				outBmp = new Bitmap(bounds.Width-1,bounds.Height-1,render->GetPixelFormat());
				outGfx = Graphics::FromImage(outBmp);

				my.Translate((REAL)-bounds.X,(REAL)-bounds.Y);
				selee->Transform(&my);
				outGfx->SetClip(selee);

				outGfx->DrawImage(
					render,
					Rect(0,0,outBmp->GetWidth(),outBmp->GetHeight()),
					bounds.X,
					bounds.Y,
					outBmp->GetWidth(),
					outBmp->GetHeight(),
					UnitPixel
					);

				delete render;
				break;
		}
		delete selee;
		delete selclr;

		if( outGfx != NULL )
			delete outGfx;

		return outBmp;
	}
	return NULL;
}

void Workspace::setSelectedLayer(Frame *layer)
{
	this->selectedLayers = new List<Frame>();
	this->selectedLayers->add(layer);
}

void Workspace::addSelectedLayer(Frame *layer)
{
	this->selectedLayers->add(layer);
}

ChildCore *Workspace::getChicore()
{
	return this->chicore;
}

HistoryControler *Workspace::getHistory()
{
	return this->history;
}