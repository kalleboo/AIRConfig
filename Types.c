
typedef struct {
	FSSpec			outFileSpec;
	FSSpec			inFileSpec;
	
	char *			inputString;
	long			inputStringLength;
	
	long			totalEntries;
	long 			totalTextLength;
	
	Str255			resourceName;
	short			resourceId;
} AppState;


typedef struct {
	Boolean			hasSeenHelp;
} AppPrefs;


typedef struct {
	DialogPtr		ptr;
	Handle			inputStatus;
	Handle			outputStatus;
	ControlHandle	doItButton;
	
} MainWindow;

