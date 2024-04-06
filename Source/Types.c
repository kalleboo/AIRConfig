
typedef struct {
	FSSpec			outFileSpec;
	FSSpec			inFileSpec;
	
	char *			inputString;
	long			inputStringLength;
	
	long			totalEntries;
	long 			totalTextLength;
	
	Str255			resourceName;
	short			resourceId;
	
	Boolean			runningHeadless;
} AppState;


typedef struct {
	Boolean			hasSeenHelp;
	Boolean			headlessMode;
} AppPrefs;


typedef struct {
	DialogPtr		ptr;
	
	Handle			inputStatus;
	ControlHandle	inputButton;
	
	Handle			outputStatus;
	ControlHandle	outputButton;
	
	ControlHandle	doItButton;
	
} MainWindow;
