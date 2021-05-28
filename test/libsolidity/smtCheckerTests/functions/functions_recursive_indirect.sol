contract C
{
	uint a;
	function f() public {
		if (a > 0)
		{
			a = a - 1;
			g();
		}
		else
			assert(a == 0);
	}
	function g() public {
		if (a > 0)
		{
			a = a - 1;
			f();
		}
		else
			assert(a == 0);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 0: (0-213): Contract invariants for :C:\n(a <= 0)\n
