
typedef struct {
	Boolean			inputIsValid;
	Boolean			outputIsValid;
	
	Handle			loadedInputString;
	
	long			totalEntries;
	long 			totalTextLength;
	
	Str255			resourceName;
	short			resourceId;
	
	Boolean			runningHeadless;
} AppState;


typedef struct {
	Boolean			hasSeenHelp;
	
	Boolean			headlessMode;
	
	short			inputSource;
	AliasHandle		inputFileAlias;
	AliasHandle		outputFileAlias;
	Str255			inputURL;
} AppPrefs;


typedef struct {
	DialogPtr		ptr;
	
	Handle			inputStatus;
	ControlHandle	inputButton;
	
	Handle			outputStatus;
	ControlHandle	outputButton;
	
	ControlHandle	doItButton;
	
} MainWindow;
