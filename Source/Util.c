pascal void DrawDefaultButtonUserItem(WindowPtr theWindow, short itemNo);
void StringInsert(Str255 baseString, Str255 subsString, Str255 resultString);
pascal void MyDrawRect(WindowPtr theWindow, short itemNo);
short isKeyPressed(unsigned short k);


#pragma segment Main
Boolean isOptionKeyPressed() {
	return isKeyPressed(0x3A);
}


#pragma segment Main
short isKeyPressed(unsigned short k) {
	unsigned char km[16];
	
	GetKeys((unsigned long*) km);
	return ( ( km[k>>3] >> (k & 7) ) & 1);
}



#pragma segment Main
void StringInsert(Str255 baseString, Str255 subsString, Str255 resultString) {
	Handle 	baseHandle;
	Handle 	subsHandle;
	Str15 	keyStr = "\p^0";
	long	sizeL;
	long	length;
	
	sizeL = baseString[0];
	PtrToHand(&baseString[1], &baseHandle, sizeL);
	
	sizeL = subsString[0];
	PtrToHand(&subsString[1], &subsHandle, sizeL);
	
	ReplaceText(baseHandle, subsHandle, keyStr);
	
	length = GetHandleSize(baseHandle);
	
	if (length > 254) {
		length = 254;
	}
	
	//construct Pascal string
	resultString[0] = (char) length;
	BlockMove(*baseHandle, &resultString[1], length);
	
	DisposeHandle(baseHandle);
	DisposeHandle(subsHandle);
}


#pragma segment Main
pascal void MyDrawRect(WindowPtr theWindow, short itemNo) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	
	GetDialogItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	FrameRect(&iRect);
}



#pragma segment Main
pascal void DrawDefaultButtonUserItem(WindowPtr theWindow, short itemNo) {
	short		iType;
	Handle		iHandle;
	Rect		iRect;
	short		itemX = itemNo;
	
	GetDialogItem(theWindow, ok, &iType, &iHandle, &iRect);
	iRect.top = iRect.top - 4;
	iRect.left = iRect.left - 4;
	iRect.bottom = iRect.bottom + 4;
	iRect.right = iRect.right + 4;
	
	PenSize(3,3);
	FrameRoundRect(&iRect, 16, 16);
	PenSize(1,1);
}