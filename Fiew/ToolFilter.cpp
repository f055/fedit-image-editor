#include "stdafx.h"
#include "Core.h"

ToolFilter::ToolFilter() : Tool(new FwCHAR(TOOL_FILTER),NULL,FILTER)
{
	this->lastFilterId = ID_FILTER_BLUR;
	this->lastFilterValue = 1;

	this->lastActivate = false;
	this->isLastActivate = false;

	this->lastBw = false;
	this->lastAlpha = true;

	int size = 5;
	this->lastCustomMatrix.matrix = ToolFilter::allocMatrix(size,size);
	this->lastCustomMatrix.bias = 0;
	this->lastCustomMatrix.division = 1;
	this->lastCustomMatrix.mxw = size;
	this->lastCustomMatrix.mxh = size;
}

ToolFilter::~ToolFilter()
{

}

void ToolFilter::activate()
{
	ChildCore *child = this->core->getActiveChild();
	if( child != NULL ){
		if( this->lastActivate == true && this->isLastActivate == false ){
			this->lastActivate = false;
			return;
		}

		ToolFilter::Info fi;
		Bitmap *source = NULL;
		if( child->getWorkspace()->getSelection() != NULL ){
			source = child->getWorkspace()->getSelectedProjection(NULL,COPCOP);
		}
		else {
			source = child->getWorkspace()->getSelectedLayer()->getRender();
			source = source->Clone(Rect(0,0,source->GetWidth(),source->GetHeight()),source->GetPixelFormat());
		}				

		fi.bmpSource = source;
		fi.bmpEffect = NULL;
		fi.bw = this->lastBw;
		fi.bwalpha = (this->lastAlpha &&
					  this->lastFilterId != ID_FILTER_OLDSTONE &&
					  this->lastFilterId != ID_FILTER_EDGETRACE );
		fi.smooth = true;
		fi.edgeTrace = (this->lastFilterId == ID_FILTER_EDGETRACE);

		fi.filterId = this->lastFilterId;
		fi.filterValue = this->lastFilterValue;
		fi.filterByValue = (this->lastFilterId != ID_FILTER_CUSTOM);

		fi.minVal = 0;
		switch(fi.filterId){
			case ID_FILTER_SHARPEN:
				fi.maxVal = SHARPENMAX;
				break;
			default:
				fi.maxVal = FILTERMAX;
				break;
		}

		if( fi.filterByValue == true )
			fi.matrix = ToolFilter::allocateMatrix(fi.filterId,fi.filterValue);
		else
			fi.matrix = this->lastCustomMatrix;

		int result = NO;
		if( this->lastActivate == false ){
			result = this->core->getDialogs()->showDialog(
														(LPCTSTR)IDD_FIL,
														(DLGPROC)Dialogs::processDlg_Fil,
														(LPARAM)&fi
														);
		}
		else {
			this->lastActivate = false;
			result = YES;
		}
		if( result == YES ){

			Point shift(0,0);
			Rect *bounds = NULL;

			if( child->getWorkspace()->getSelection() != NULL ){
				bounds = new Rect(0,0,0,0);
				child->getWorkspace()->getSelection()->GetBounds(bounds);

				fi.bmpSource = child->getWorkspace()->getSelectedLayer()->getRender();
			}
			else {
				int exw = (int)( floor(fi.matrix.mxw / 2.0) + 1 );
				int exh = (int)( floor(fi.matrix.mxh / 2.0) + 1 );

				shift.X = -exw;
				shift.Y = -exh;

				Bitmap *newsource = new Bitmap(
					source->GetWidth() + 2 * exw,
					source->GetHeight() + 2 * exh,
					source->GetPixelFormat());
				Graphics *g = Graphics::FromImage(newsource);
				g->DrawImage(source,exw,exh,source->GetWidth(),source->GetHeight());

				delete g;
				delete source;

				source = newsource;
				fi.bmpSource = newsource;
			}
			if( fi.filterByValue == true )
				this->lastFilterValue = fi.filterValue;
			else
				this->lastCustomMatrix = fi.matrix;
			this->lastBw = fi.bw;
			this->lastAlpha = fi.bwalpha;

			Rect newbounds = ToolFilter::applyFilter(&fi,bounds);

			if( child->getWorkspace()->getSelection() == NULL ){
				bounds = &newbounds;
				bounds->X += shift.X;
				bounds->Y += shift.Y;
			}

			child->getWorkspace()->getSelectedLayer()->applyEffect(
				fi.bmpEffect,
				bounds,
				this,
				fi.filterId);
			delete fi.bmpEffect;

			this->isLastActivate = true;

			child->getWorkspace()->updateToolws();
			child->getWorkspace()->update();
		}
		if( source != NULL )
			delete source;
	}
}

void ToolFilter::activateAgain()
{
	this->lastActivate = true;
	this->activate();
}

bool ToolFilter::canActivateAgain()
{
	return this->isLastActivate;
}

void ToolFilter::setFilterId(int id)
{
	this->lastFilterId = id;
}
int ToolFilter::getFilterId()
{
	return this->lastFilterId;
}

double ToolFilter::calcMatrixWeight(ToolFilter::Matrix *matrix)
{
	double weight = 0;

	/*
	int w = floor(matrix->mxw / 2.0);
	int h = floor(matrix->mxh / 2.0);

	for(int x = max(w - 2,0); x < min(w + 2,matrix->mxw); x++)
		for(int y = max(h - 2,0); y < min(h + 2,matrix->mxh); y++)
			weight += matrix->matrix[x][y];
			*/

	for(int x = 0; x < matrix->mxw; x++)
		for(int y = 0; y < matrix->mxh; y++)
			weight += matrix->matrix[x][y];

	return weight;
}

Rect ToolFilter::applyFilter(ToolFilter::Info *fi, Rect *clip, bool once)
{
	Core::self->getGui()->setCursor(CURSOR_WAIT);

	int w = (int)( floor(fi->matrix.mxw / 2.0) );
	int h = (int)( floor(fi->matrix.mxh / 2.0) );

	int exw = fi->matrix.mxw + 1;
	int exh = fi->matrix.mxh + 1;

	UINT *src0, *bmp0;
	BitmapData srcData, bmpData;

	Bitmap *src = fi->bmpSource;
	Rect srcRect(0,0,src->GetWidth(),src->GetHeight());

	if( clip == NULL ){
		clip = &srcRect;
	}
	else {
		int maxx = max(clip->X - exw,0);
		int maxy = max(clip->Y - exh,0);
		int ext = 2;

		srcRect = Rect(
			maxx,
			maxy,
			min(clip->Width + ext * exw,(int)src->GetWidth() - maxx),
			min(clip->Height + ext * exh,(int)src->GetHeight() - maxy)
			);
	}

	int bmpWidth = clip->Width;
	int bmpHeight = clip->Height;
	if( fi->smooth == true ){
		bmpWidth += 4 * exw;
		bmpHeight += 4 * exh;
	}

	Bitmap *bmp = new Bitmap(
		bmpWidth,
		bmpHeight,
		src->GetPixelFormat()
		);
	Rect bmpRect(0,0,bmp->GetWidth(),bmp->GetHeight());

	src->LockBits(
			&srcRect,
			ImageLockModeRead,
			src->GetPixelFormat(),
			&srcData
			);
	bmp->LockBits(
			&bmpRect,
			ImageLockModeWrite,
			bmp->GetPixelFormat(),
			&bmpData
			);
	src0 = (UINT *)srcData.Scan0;
	bmp0 = (UINT *)bmpData.Scan0;

	int srcWidth = srcData.Width;
	int srcHeight = srcData.Height;

	for( int x = 0; x < bmpWidth; x++ ){
		for( int y = 0; y < bmpHeight; y++ ){
			bmp0[y * bmpData.Stride / 4 + x] = ToolFilter::filterPixel(fi,&srcData,x,y,w,h); 
		}
	}
	if( fi->edgeTrace == true && fi->filterValue > 0 ){
		ToolFilter::Matrix oldMatrix = fi->matrix;

		fi->matrix = ToolFilter::allocMatrixEdgetrace(fi->filterValue,1);
		for( int x = 0; x < bmpWidth; x++ ){
			for( int y = 0; y < bmpHeight; y++ ){
				bmp0[y * bmpData.Stride / 4 + x] += ToolFilter::filterPixel(fi,&srcData,x,y,w,h); 
			}
		}
		fi->matrix = ToolFilter::allocMatrixEdgetrace(fi->filterValue,2);
		for( int x = 0; x < bmpWidth; x++ ){
			for( int y = 0; y < bmpHeight; y++ ){
				bmp0[y * bmpData.Stride / 4 + x] += ToolFilter::filterPixel(fi,&srcData,x,y,w,h); 
			}
		}

		fi->matrix = oldMatrix;
	}
	src->UnlockBits(&srcData);
	bmp->UnlockBits(&bmpData);

	fi->bmpEffect = bmp;

	int refils = 0;
	switch(fi->filterId){
		case ID_FILTER_SHARPEN:
			if( fi->filterValue > SHARPENMAX/2 )
				refils = fi->filterValue - SHARPENMAX/2;
			break;
		case ID_FILTER_GAUSSIANBLUR:
			if( fi->filterValue > GAUSSMAX )
				refils = fi->filterValue - GAUSSMAX;
			break;
	}
	Bitmap *source = fi->bmpSource;

	if( refils > 0 && once == false ){
		for(int i = 0; i < refils; i++ ){
			fi->bmpSource = fi->bmpEffect;
			fi->bmpEffect = NULL;

			ToolFilter::applyFilter(fi,NULL,true);

			delete fi->bmpSource;
		}
	}
	fi->bmpSource = source;

	Rect gridClip(clip->X,clip->Y,clip->Width,clip->Height);

	clip->X -= exw;
	clip->Y -= exh;

	Core::self->getGui()->setCursor();

	return gridClip;
}

INT ToolFilter::scalePixel(RGBCOLOR color, bool alpha)
{
	return (int)( (color.R + color.G + color.B + ((alpha == true) ? color.A : 0)) / ((alpha == true) ? 4.0 : 3.0) );
}

UINT ToolFilter::getPixel(BitmapData *bmp, int x, int y)
{
	if( x >= 0 && y >= 0 && x < (int)bmp->Width && y < (int)bmp->Height ){
		UINT *src0 = (UINT *)bmp->Scan0;
		return src0[y * bmp->Stride / 4 + x];
	}
	return NULL;
}

UINT ToolFilter::filterPixel(ToolFilter::Info *fi, BitmapData *bmp, int x, int y, int w, int h)
{
	ToolFilter::Matrix *matrix = &fi->matrix;
	bool edgeTrace = fi->edgeTrace;
	bool bw = fi->bw;
	bool bwa = fi->bwalpha;

	double division = matrix->division;
	double bias = matrix->bias;

	int val = 0;
	int R,G,B,A;
	R = G = B = A = 0;

	for( int i = -w; i <= w; i++ )
	{
		int bval = 0;
		int lastval = 0;
		int lR, lG, lB, lA, bR, bG, bB, bA;
		lR = lG = lB = lA = 0;
		bR = bG = bB = bA = 0;

		for( int j = -h; j <= h; j++ )
		{
			int xx = x + i;
			int yy = y + j;

			if( (xx < 0 || yy < 0 ||
				xx >= (int)bmp->Width ||
				yy >= (int)bmp->Height) && 
				fi->smooth == false )
			{
				if( bw == false )
				{
					R += bR;
					G += bG;
					B += bB;
					A += bA;
				}
				else
					val += bval;
				continue;
			}
			RGBCOLOR pick = Core::UINTtoRGBCOLOR( ToolFilter::getPixel(bmp,xx,yy) );
			int factor = matrix->matrix[w+i][h+j];

			if( bw == false )
			{
				bR = pick.R;
				bG = pick.G;
				bB = pick.B;
				bA = pick.A;
				lR = pick.R * factor;
				lG = pick.G * factor;
				lB = pick.B * factor;
				lA = pick.A * factor;
				R += lR;
				G += lG;
				B += lB;
				A += lA;
			}
			else 
			{
				bval = ToolFilter::scalePixel(pick);
				lastval = (bval * factor);
				val += lastval;
			}
		}
	}
	double div = ToolFilter::calcMatrixWeight(matrix);
	if( div == 0 ) div = 1;

	if( bw == false )
	{
		double tR = R / div;
		double tG = G / div;
		double tB = B / div;
		double tA = A / div;
		R = (int)((tR / division) + bias);
		G = (int)((tG / division) + bias);
		B = (int)((tB / division) + bias);
		A = (int)((tA / division) + bias);

		if( R < 0 ) R = 0;
		if( R > 255 ) R = 255;
		if( G < 0 ) G = 0;
		if( G > 255 ) G = 255;
		if( B < 0 ) B = 0;
		if( B > 255 ) B = 255;
	}
	else
	{
		double temp = val / div;
		val = (int)((temp / division) + bias);

		min( max(val,255),0 );
	}

	if( edgeTrace == true )
	{
		if( bw == false )
		{
			R = 255 - R;
			G = 255 - G;
			B = 255 - B;
		}
		else
			val = 255 - val;
	}

	if( bw == true )
		return Core::RGBtoUINT(val,val,val,((bwa == true) ? val : 255));

	return Core::RGBtoUINT(R,G,B,((bwa == true) ? A : 255));
}

int **ToolFilter::allocMatrix(int mxw, int mxh, int set)
{
	int **matrix = (int **)malloc(mxw * sizeof(int *));
	for( int i = 0; i < mxw; i++ )
		matrix[i] = (int *)malloc(mxh * sizeof(int));

	for( int i = 0; i < mxw; i++ )
		for( int j = 0; j < mxh; j++ )
			matrix[i][j] = set;

	int i = (int)floor(mxw / 2.0);
	int j = (int)floor(mxh / 2.0);
	matrix[i][j] = 1;

	return matrix;
}

ToolFilter::Matrix ToolFilter::allocMatrixGeneric()
{
	int size = 3;

	int **matrix = ToolFilter::allocMatrix(size,size);

	for( int x = 0; x < size; x++ )
		for( int y = 0; y < size; y++ )
			matrix[x][y] = 0;
	matrix[1][1] = 1;

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}

ToolFilter::Matrix ToolFilter::allocMatrixBlur(int value)
{
	int size = 2 * value + 1;
	int factor = max(2 * value - 1 - value, 1);

	int **matrix = ToolFilter::allocMatrix(size,size);

	for( int x = 0; x < size; x++ )
		for( int y = 0; y < size; y++ )
			matrix[x][y] = factor;

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}

ToolFilter::Matrix ToolFilter::allocMatrixBlurGauss(int value)
{
	value = min(value,GAUSSMAX);

	int size = 2 * value + 1;
	int factor = max(2 * value - 1 - value, 1);

	int **matrix = ToolFilter::allocMatrix(size,size);

	matrix[0][0] = 0;
	if( value > 0 ){
		for( int x = 1; x <= value; x++ )
			matrix[x][0] = max(2 * matrix[x - 1][0] , 2);
		for( int x = value + 1; x < size; x++ )
			matrix[x][0] = (int)floor(matrix[x - 1][0] / 2.0);

		for( int x = 0; x < size; x++ ){
			for( int y = 1; y <= value; y++ )
				matrix[x][y] = max(2 * matrix[x][y - 1], 2);
			for( int y = value + 1; y < size; y++ )
				matrix[x][y] = (int)floor(matrix[x][y - 1] / 2.0);
		}
	}
	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}
ToolFilter::Matrix ToolFilter::allocMatrixSharpen(int value)
{
	int size = 2 * value + 1;
	int maxval = 2 * SHARPENMAX + 1;

	int **matrix = ToolFilter::allocMatrix(3,3,-1);
	matrix[1][1] = max(maxval - size,10);

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = 3;//size;
	fmx.mxh = 3;//size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}
ToolFilter::Matrix ToolFilter::allocMatrixEdgetrace(int value, int mode)
{
	if( value == 0 )
		return ToolFilter::allocMatrixGeneric();

	int size = 2 * value + 1;

	int **matrix = ToolFilter::allocMatrix(size,size);

	switch(mode){
		case 1:
			for( int x = size - 1; x > value; x-- )
				matrix[x][value] = -value;
			break;
		case 2:
			for( int y = 0; y < value; y++ )
				matrix[value][y] = -value;
			break;
		default:
			for( int x = 0; x < value; x++ )
				matrix[x][value] = -value;
			break;
	}

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}
ToolFilter::Matrix ToolFilter::allocMatrixEmboss(int value, int mode)
{
	int size = 2 * value + 1;

	int **matrix = ToolFilter::allocMatrix(size,size);

	switch(mode){
		case ID_EMBOSS_EAST:
			for( int y = 0; y < size; y++ )
				for( int x = y; x < size - y; x++ )
					matrix[x][y] = -1;
			for( int y = size - 1; y >= 0; y-- )
				for( int x = size - y - 1; x < y + 1; x++ )
					matrix[x][y] = 1;
			break;
		case ID_EMBOSS_SOUTH:
			for( int x = 0; x < size; x++ )
				for( int y = x; y < size - x; y++ )
					matrix[x][y] = -1;
			for( int x = size - 1; x >= 0; x-- )
				for( int y = size - x - 1; y < x + 1; y++ )
					matrix[x][y] = 1;
			break;
	}

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}
ToolFilter::Matrix ToolFilter::allocMatrixHighlight(int value)
{
	int size = 2 * value + 1;

	int **matrix = ToolFilter::allocMatrix(size,size);
	matrix[0][0] = -1;
	matrix[size - 1][size - 1] = 3;
	matrix[value][value] = 0;

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}
ToolFilter::Matrix ToolFilter::allocMatrixDefocus(int value)
{
	int size = 2 * value + 1;

	int **matrix = ToolFilter::allocMatrix(size,size,1);
	matrix[value][value] = -size;

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 0;
	fmx.division = 1;

	return fmx;
}
ToolFilter::Matrix ToolFilter::allocMatrixOldstone(int value)
{
	int size = 2 * value + 1;

	int **matrix = ToolFilter::allocMatrix(size,size);

	for( int x = 0; x < size; x++ )
		for( int y = 0; y < size - x - 2; y++ )
			matrix[x][y] = -1;
	for( int x = 0; x < size; x++ )
		for( int y = size - x + 1; y < size; y++ )
			matrix[x][y] = 1;
	matrix[value][value] = 0;

	ToolFilter::Matrix fmx;

	fmx.matrix = matrix;
	fmx.mxw = size;
	fmx.mxh = size;

	fmx.bias = 90;
	fmx.division = 1;

	return fmx;
}

ToolFilter::Matrix ToolFilter::allocateMatrix(int filterId, int value)
{
	if( value == 0 )
		return ToolFilter::allocMatrixGeneric();

	switch(filterId){
		case ID_FILTER_BLUR:
			return ToolFilter::allocMatrixBlur(value);	
		case ID_FILTER_GAUSSIANBLUR:
			return ToolFilter::allocMatrixBlurGauss(value);
		case ID_FILTER_SHARPEN:
			return ToolFilter::allocMatrixSharpen(value);
		case ID_FILTER_EDGETRACE:
			return ToolFilter::allocMatrixEdgetrace(value);
		case ID_EMBOSS_EAST:
		case ID_EMBOSS_SOUTH:
		case ID_EMBOSS_SOUTHEAST:
			return ToolFilter::allocMatrixEmboss(value,filterId);
		case ID_FILTER_HIGHLIGHT:
			return ToolFilter::allocMatrixHighlight(value);
		case ID_FILTER_DEFOCUS:
			return ToolFilter::allocMatrixDefocus(value);
		case ID_FILTER_OLDSTONE:
			return ToolFilter::allocMatrixOldstone(value);
	}
	return ToolFilter::allocMatrixGeneric();
}

WCHAR *ToolFilter::getFilterName(int filterId)
{
	switch(filterId)
	{
		case ID_FILTER_BLUR:
			return FIL_BLUR;
		case ID_FILTER_GAUSSIANBLUR:
			return FIL_BLURGAUSS;
		case ID_FILTER_SHARPEN:
			return FIL_SHARPEN;
		case ID_FILTER_EDGETRACE:
			return FIL_EDGETRACE;
		case ID_EMBOSS_EAST:
			return FIL_EMBOSS_E;
		case ID_EMBOSS_SOUTH:
			return FIL_EMBOSS_S;
		case ID_EMBOSS_SOUTHEAST:
			return FIL_EMBOSS_SE;
		case ID_FILTER_HIGHLIGHT:
			return FIL_HIGHLIGHT;
		case ID_FILTER_DEFOCUS:
			return FIL_DEFOCUS;
		case ID_FILTER_OLDSTONE:
			return FIL_OLDSTONE;
		case ID_FILTER_CUSTOM:
			return FIL_CUSTOM;
	}
	return FIL_GENERIC;
}