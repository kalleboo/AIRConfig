
Handle StringInsert(Str255 baseString, Str255 subsString);
pascal void MyDrawRect(WindowPtr theWindow, short itemNo);



#pragma segment Main
size_t strlen(const char *str) {
	const char *s;
	for (s = str; *s; ++s) { }
	return s - str;
}



#pragma segment Main
Handle StringInsert(Str255 baseString, Str255 subsString) {
	Handle 	baseHandle;
	Handle 	subsHandle;
	Str15 	keyStr = "\p^0";
	long	sizeL;
	long	length;
	
	Str255	output;
	Handle 	outputHand;
	
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
	output[0] = (char) length;
	BlockMove(*baseHandle, &output[1], length);
	
	//Have to return a handle
	PtrToHand(&output, &outputHand, length + 1);
	
	//This function definitely leaks memory...
	return outputHand;
}


#pragma segment Main
pascal void MyDrawRect(WindowPtr theWindow, short itemNo) {
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	
	GetDItem(theWindow, itemNo, &iType, &iHandle, &iRect);
	FrameRect(&iRect);
}