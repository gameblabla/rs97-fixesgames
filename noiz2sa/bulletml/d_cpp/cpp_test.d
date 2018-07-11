import cpp_inter;

import stream;
import string;

extern (C) {
	int return7(D_I* i) { return 7; }
	int add(D_I* i, int x, int y) { return x+y; }
}

int main() {
	D_C* c = D_C_new_1(1);
	stdout.writeLine(string.toString(D_C_a(c)));
	D_C_delete(c);

	c = D_C_new();
	D_C_setA(c, 2);
	stdout.writeLine(string.toString(D_C_a(c)));

	D_C_f2(c, 0, 0);
	D_C_f5(c, 0,0,0,0,0);
	D_C_f9(c, 0,0,0,0,0,0,0,0,0);

	D_C_over_load_int(c, 3);
	stdout.writeLine(string.toString(D_C_a(c)));

	D_C_over_load_char(c, '4');
	stdout.writeLine(string.toString(D_C_a(c)));

	D_TC_int* tc = D_TC_int_new();
	stdout.writeLine(string.toString(D_TC_int_f(tc)));
	stdout.writeLine(string.toString(D_TC_int_tf(tc)));

	D_C_delete(c);
	D_TC_int_delete(tc);

	D_I* i = D_I_new();
	D_I_setCallbackFunc(i, &return7);
	D_I_setCallback2Func(i, &add);
	stdout.writeLine(string.toString(D_I_load_callback(i)));
	stdout.writeLine(string.toString(D_I_load_callback2(i, 3, 5)));
	D_I_delete(i);

	return 0;
}
