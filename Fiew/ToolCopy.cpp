#include "stdafx.h"
#include "Core.h"

ToolCopy::ToolCopy(int mode) : Tool(new FwCHAR(),NULL,mode)
{
	this->mode = mode;
	this->noclipboard = false;

	delete this->name;
	switch(this->mode){
		case COPCOP:
			this->name = new FwCHAR(TOOL_COPY);
			break;
		case COPCPM:
			this->name = new FwCHAR(TOOL_COPM);
			break;
		case COPCUT:
			this->name = new FwCHAR(TOOL_CUT);
			break;
		case COPPAS:
			this->name = new FwCHAR(TOOL_PASTE);
			break;
		case COPCLR:
			this->name = new FwCHAR(TOOL_CLEAR);
			break;
	}
}

ToolCopy::~ToolCopy()
{

}

void ToolCopy::activate()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		Bitmap *clipboardBmp = NULL;

		switch(this->mode){
			case COPCOP:
			case COPCPM:
			case COPCUT:
			case COPCLR:
				if( child->getWorkspace()->getSelection() != NULL ){
					clipboardBmp =
						child->getWorkspace()->getSelectedProjection(NULL,this->mode);

					if( clipboardBmp != NULL && this->noclipboard == false ){
						Core::setClipboardBitmap(clipboardBmp);
						delete clipboardBmp;
					}
				}
				else {
					this->core->messageBox_Error(TOOL_COPY_MESSAGE);
				}
				break;
			case COPPAS:
				if( this->noclipboard == false )
					clipboardBmp = Core::getClipboardBitmap();
				if( clipboardBmp != NULL ){
					child->getWorkspace()->setSelectedProjection(clipboardBmp,this->mode);
				}
				break;
		}
	}
	this->noclipboard = false;
}

void ToolCopy::setNoclipboard()
{
	this->noclipboard = true;
}