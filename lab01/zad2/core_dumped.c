int main()
{
	int x = 5;
	int i = 0;
	int f = 1;

	for (; i < 10; ++i)
	{
		f = i / x;
		--x;
	}

	x = f;

	return 0;
}
