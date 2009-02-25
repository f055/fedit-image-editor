/*
HistoryElements.cpp
Each element is responsible for undo and redo actions on a specified
process of editing. All elements inherit from HistoryElement and
follow the same interface, using set() methods for supplying data
and undoAct() and redoAct() to perform going backword and forward
with acitons
*/

#include "stdafx.h"
#include "Core.h"

HistoryElement::HistoryElement(Workspace *wspc, Frame *frame, Tool *tool)
{
	this->wspc = wspc;
	this->frame = frame;
	this->tool = tool;

	this->canUndo = true;
	this->canRedo = false;

	this->name = NULL;
}

HistoryElement::~HistoryElement()
{

}

bool HistoryElement::undo(){
	bool result = this->canUndo;

	if( result == true )
		this->undoAct();
	this->boolUndo();

	return result;
}
bool HistoryElement::redo(){
	bool result = this->canRedo;

	if( result == true )
		this->redoAct();
	this->boolRedo();

	return result;
}

void HistoryElement::undoAct(){ }
void HistoryElement::redoAct(){ }

void HistoryElement::boolUndo()
{
	this->canUndo = false;
	this->canRedo = true;
}
void HistoryElement::boolRedo()
{
	this->canUndo = true;
	this->canRedo = false;
}

bool HistoryElement::canUndone()
{
	return this->canUndo;
}
bool HistoryElement::canRedone()
{
	return this->canRedo;
}

FwCHAR *HistoryElement::getName()
{
	if( this->tool != NULL )
		return this->tool->getName();
	return NULL;
}

WORD HistoryElement::getId()
{
	if( this->tool != NULL )
		return this->tool->getId();
	return NULL;
}

Workspace *HistoryElement::getWorkspace()
{
	return this->wspc;
}
Frame *HistoryElement::getFrame()
{
	return this->frame;
}
Tool *HistoryElement::getTool()
{
	return this->tool;
}

HistoryBlankElement::HistoryBlankElement() : HistoryElement(NULL,NULL,NULL)
{
	this->name = new FwCHAR(TOOLWBOX_HISTORY_FIRST);
}
HistoryBlankElement::~HistoryBlankElement()
{ 
	delete this->name;
}

bool HistoryBlankElement::undo(){ return false; }
bool HistoryBlankElement::redo(){ return false; }

FwCHAR *HistoryBlankElement::getName()
{
	return this->name;
}

WORD HistoryBlankElement::getId()
{
	return NULL;
}

/* History Tool Elements */

/* Move Tool */
HEToolMove::HEToolMove(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																				   frame,
																				   tool)
{
	this->dx = NULL;
	this->dy = NULL;
}

HEToolMove::~HEToolMove()
{

}

void HEToolMove::set(int dx, int dy)
{
	this->dx = dx;
	this->dy = dy;
}

void HEToolMove::undoAct()
{
	this->frame->addXY(-this->dx,-this->dy,true);

	if( this->wspc->getSelection() != NULL ){
		Matrix off;
		off.Translate(-this->dx,-this->dy);
		this->wspc->getSelection()->Transform(&off);
	}
}

void HEToolMove::redoAct()
{
	this->frame->addXY(this->dx,this->dy,true);

	if( this->wspc->getSelection() != NULL ){
		Matrix off;
		off.Translate(this->dx,this->dy);
		this->wspc->getSelection()->Transform(&off);
	}
}

/* Drawing Tool */
HEToolDrawing::HEToolDrawing(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																						 frame,
																						 tool)
{
	this->before = NULL;
	this->after = NULL;
	this->target = Rect(0,0,0,0);

	this->toolId = FERROR;
	this->name = NULL;
}

HEToolDrawing::~HEToolDrawing()
{
	if( this->before != NULL )
		delete this->before;
	if( this->after != NULL )
		delete this->after;
	if( this->name != NULL )
		delete this->name;
}

void HEToolDrawing::set(Bitmap *before, Bitmap *after, Rect target)
{
	this->before = before;
	this->after = after;

	this->target = target;
}

void HEToolDrawing::setToolId(int id)
{
	this->toolId = id;
}

FwCHAR *HEToolDrawing::getName()
{
	if( this->tool != NULL && this->toolId == FERROR ){
		return this->tool->getName();
	}
	else {
		if( this->name != NULL )
			delete this->name;
		return this->name = new FwCHAR(ToolFilter::getFilterName(this->toolId));
	}
	return NULL;
}

void HEToolDrawing::undoAct()
{
	this->drawImage(this->before);
}

void HEToolDrawing::redoAct()
{
	this->drawImage(this->after);
}

void HEToolDrawing::drawImage(Bitmap *image)
{
	this->frame->getGraphics()->SetClip(this->target);
	this->frame->getGraphics()->Clear(NULL);

	if( image != NULL ){
		this->frame->getGraphics()->DrawImage(
			image,
			target,
			0,0,
			target.Width,
			target.Height,
			UnitPixel
			);
	}
	this->frame->getGraphics()->ResetClip();
}

/* Selecting Tool */
HEToolSelecting::HEToolSelecting(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																							 frame,
																							 tool)
{
	this->seleeBefore = NULL;
	this->seleeAfter = NULL;

	this->deseleeName = new FwCHAR(TOOLWBOX_HISTORY_DESELEE);
	this->name = this->tool->getName();
}

HEToolSelecting::~HEToolSelecting()
{
	if( this->seleeBefore != NULL )
		delete this->seleeBefore;
	if( this->seleeAfter != NULL )
		delete this->seleeAfter;

	delete this->deseleeName;
}

void HEToolSelecting::set(GraphicsPath *pathBefore, GraphicsPath *pathAfter)
{
	this->seleeBefore = (pathBefore != NULL ) ? pathBefore->Clone() : NULL;
	this->seleeAfter = (pathAfter != NULL ) ? pathAfter->Clone() : NULL;
}

void HEToolSelecting::setUpdate(GraphicsPath *pathAfter)
{
	if( pathAfter != NULL ){
		if( this->seleeAfter != NULL )
			delete this->seleeAfter;

		this->seleeAfter = pathAfter->Clone();
	}

}

FwCHAR *HEToolSelecting::getName()
{
	if( this->seleeAfter == NULL )
		return this->deseleeName;
	return this->name;
}

void HEToolSelecting::undoAct()
{
	this->wspc->setSelection(
		(this->seleeBefore != NULL ) ? this->seleeBefore->Clone() : NULL
		);
}

void HEToolSelecting::redoAct()
{
	this->wspc->setSelection(
		(this->seleeAfter != NULL ) ? this->seleeAfter->Clone() : NULL
		);
}

/* Crop Tool */
HEToolCrop::HEToolCrop(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																				   frame,
																				   tool)
{
	SetRect(&this->canvasChange,0,0,0,0);
}

HEToolCrop::~HEToolCrop()
{

}

void HEToolCrop::set(RECT canvas)
{
	RECT change;
	SetRect(
		&change,
		canvas.left,
		canvas.top,
		canvas.right - this->wspc->getPxWidth(),
		canvas.bottom - this->wspc->getPxHeight()
		);

	this->canvasChange = change;
}

void HEToolCrop::undoAct()
{
	RECT canvas;
	SetRect(
		&canvas,
		- this->canvasChange.left,
		- this->canvasChange.top,
		this->wspc->getPxWidth() - this->canvasChange.right,
		this->wspc->getPxHeight() - this->canvasChange.bottom
		);

	this->wspc->resizeCanvas(canvas,true);
}

void HEToolCrop::redoAct()
{
	RECT canvas;
	SetRect(
		&canvas,
		this->canvasChange.left,
		this->canvasChange.top,
		this->wspc->getPxWidth() + this->canvasChange.right,
		this->wspc->getPxHeight() + this->canvasChange.bottom
		);

	this->wspc->resizeCanvas(canvas,true);
}

/* Resize Tool */
HEToolResize::HEToolResize(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																					   frame,
																					   tool)
{
	this->layersBefore = NULL;
	this->layersAfter = NULL;

	this->name = new FwCHAR(TOOLWBOX_HISTORY_RESIZE);
}

HEToolResize::~HEToolResize()
{
	if( this->canRedo == true )
		if( this->layersAfter != NULL )
			delete this->layersAfter;
	if( this->canUndo == true )
		if( this->layersBefore != NULL )
			delete this->layersBefore;
	
	delete this->name;
}

void HEToolResize::set(List<Frame> *layersBefore, List<Frame> *layersAfter, Rect sizeBefore, Rect sizeAfter)
{
	this->layersBefore = layersBefore;
	this->layersAfter = layersAfter;

	this->sizeBefore = sizeBefore;
	this->sizeAfter = sizeAfter;
}

FwCHAR *HEToolResize::getName()
{
	return this->name;
}

void HEToolResize::undoAct()
{
	this->wspc->setLayers(this->layersBefore);
	this->wspc->setPxSize(this->sizeBefore);

	this->wspc->getChicore()->setText();
}

void HEToolResize::redoAct()
{
	this->wspc->setLayers(this->layersAfter);
	this->wspc->setPxSize(this->sizeAfter);

	this->wspc->getChicore()->setText();
}

/* Text Rasterize Tool */
HEToolRaster::HEToolRaster(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																					   frame,
																					   tool)
{
	this->beforeRaster = NULL;
	this->afterRaster = frame;
}

HEToolRaster::~HEToolRaster()
{

}

void HEToolRaster::set(Frame *beforeRaster, Frame *afterRaster)
{
	this->beforeRaster = beforeRaster;
	this->afterRaster = afterRaster;
}


void HEToolRaster::undoAct()
{
	this->wspc->getLayers()->gotoThat(this->afterRaster);

	this->wspc->getLayers()->swapThat(this->beforeRaster);
	this->wspc->setSelectedLayer(this->beforeRaster);
}

void HEToolRaster::redoAct()
{
	this->wspc->getLayers()->gotoThat(this->beforeRaster);

	this->wspc->getLayers()->swapThat(this->afterRaster);
	this->wspc->setSelectedLayer(this->afterRaster);
}

/* Manage Frame Tool */
HEToolManageFrame::HEToolManageFrame(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																								 frame,
																								 tool)
{
	this->mode = NULL;
	this->afterLayer = NULL;

	this->nameAdd = new FwCHAR(TOOLWBOX_HISTORY_FRAME_ADD);
	this->nameText = new FwCHAR(TOOLWBOX_HISTORY_FRAME_TEXT);
	this->nameRemove = new FwCHAR(TOOLWBOX_HISTORY_FRAME_REMOVE);
}

HEToolManageFrame::~HEToolManageFrame()
{
	delete this->nameAdd;
	delete this->nameText;
	delete this->nameRemove;
}

void HEToolManageFrame::set(Frame *afterLayer, int mode)
{
	this->mode = mode;
	this->afterLayer = afterLayer;
}

FwCHAR *HEToolManageFrame::getName()
{
	if( this->tool == NULL ){
		switch(this->mode){
			case ADD:
				return this->nameAdd;
			case REMOVE:
				return this->nameRemove;
			default:
				if( frame->getType() == FRM_TEXT )
					return this->nameText;
				return this->name;
		}
	}
	return this->tool->getName();
}

void HEToolManageFrame::undoAct()
{
	switch(this->mode){
		case ADD:
			this->wspc->deleteLayer(this->frame,true);
			break;
		case REMOVE:
			this->wspc->addLayerAfter(this->frame,this->afterLayer,true);
			break;
	}
}

void HEToolManageFrame::redoAct()
{
	switch(this->mode){
		case ADD:
			this->wspc->addLayerAfter(this->frame,this->afterLayer,true);
			break;
		case REMOVE:
			this->wspc->deleteLayer(this->frame,true);
			break;
	}
}

HEToolManageFrameOrder::HEToolManageFrameOrder(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																										   frame,
																										   tool)
{
	this->move = NULL;
	this->name = new FwCHAR(TOOLWBOX_HISTORY_ORDER);
}

HEToolManageFrameOrder::~HEToolManageFrameOrder()
{
	delete this->name;
}

void HEToolManageFrameOrder::set(int movement)
{
	this->move = movement;
}

FwCHAR *HEToolManageFrameOrder::getName()
{
	return this->name;
}

void HEToolManageFrameOrder::undoAct()
{
	this->wspc->getLayers()->gotoThat(this->frame);
	if( this->move == TOP )
		this->wspc->getLayers()->moveThatLeft();
	else
		this->wspc->getLayers()->moveThatRight();
}

void HEToolManageFrameOrder::redoAct()
{
	this->wspc->getLayers()->gotoThat(this->frame);
	if( this->move == BOT )
		this->wspc->getLayers()->moveThatLeft();
	else
		this->wspc->getLayers()->moveThatRight();
}


/* Manage Text Tool */
HEToolManageText::HEToolManageText(Workspace *wspc, Frame *frame, Tool *tool) : HistoryElement(wspc,
																							   frame,
																							   tool)
{
	this->frameText = (FrameText *)frame;

	this->textBefore = this->set(NULL,NULL,NULL,NULL,NULL,Point(0,0),Color(0,0,0,0),false);
	this->textAfter = this->set(NULL,NULL,NULL,NULL,NULL,Point(0,0),Color(0,0,0,0),false);
}

HEToolManageText::~HEToolManageText()
{
	this->freeTextState(this->textBefore);
	this->freeTextState(this->textAfter);
}

HEToolManageText::TextState HEToolManageText::set(FwCHAR *text,
												  FontFamily *textFontFamily,
												  StringFormat *textStringFormat,
												  INT textFontStyle,
												  REAL textFontSize,
												  Point org,
												  Color color,
												  bool isAA)
{
	HEToolManageText::TextState state;
	state.color = color;
	state.isAA = isAA;
	state.org = org;
	state.text = text;
	state.textFontFamily = textFontFamily;
	state.textFontSize = textFontSize;
	state.textFontStyle = textFontStyle;
	state.textStringFormat = textStringFormat;
	
	return state;
}

void HEToolManageText::setBefore(HEToolManageText::TextState before)
{
	this->textBefore = before;
}

void HEToolManageText::setAfter(HEToolManageText::TextState after)
{
	this->textAfter = after;
}

void HEToolManageText::undoAct()
{
	this->setText(this->textBefore);
}

void HEToolManageText::redoAct()
{
	this->setText(this->textAfter);
}

void HEToolManageText::setText(HEToolManageText::TextState state)
{
	this->frameText->setText(
		(state.text != NULL) ? new FwCHAR(state.text->toWCHAR()) : NULL,
		(state.textFontFamily != NULL) ? state.textFontFamily->Clone() : NULL,
		(state.textStringFormat != NULL) ? state.textStringFormat->Clone() : NULL,
		state.textFontStyle,
		state.textFontSize,
		state.org,
		state.color,
		false
		);
}

void HEToolManageText::freeTextState(HEToolManageText::TextState state)
{
	if( state.text != NULL )
		delete state.text;
	if( state.textFontFamily != NULL )
		delete state.textFontFamily;
	if( state.textStringFormat != NULL )
		delete state.textStringFormat;
}
