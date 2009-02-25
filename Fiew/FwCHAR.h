class FwCHAR
{
private:
	WCHAR *string;

public:
	FwCHAR();
	FwCHAR(int source);
	FwCHAR(WCHAR *source);
	~FwCHAR();

	void mergeWith(WCHAR *source);
	void mergeWith(FwCHAR *source);

	void getFolderFrom(FwCHAR *source);
	void getFilenameFrom(FwCHAR *source);

	WCHAR *toWCHAR();
	char *tochar();

	bool equals(FwCHAR *source);
	void stripBraces();
	void stripExtension();

	int toLength();
	int toMaxLineLength();
	int toLines();

	static int compare(FwCHAR *string1, FwCHAR *string2);

private:
};