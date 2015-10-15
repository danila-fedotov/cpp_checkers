struct way
{
	int first;
	int steps;
};
struct walking
{
	int el[12];
	int fight;
	int x, y, z;
	walking()
	{
		for (int i=0;i<12;i++) el[i]=0;
		fight=0;
		x=0;
		y=0;
		z=1;
	}
};
struct select
{
	int ix;
	int i;
	int j;
	select()
	{
		ix=-1;
		i=0;
		j=0;
	}
};