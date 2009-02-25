class Ftyper
{
public:
	Ftyper();
	~Ftyper();

	static int getMime(FwCHAR *path);
	static int getMime(File *file);

	static int getType(FwCHAR *path);
	static int getType(File *file);
};