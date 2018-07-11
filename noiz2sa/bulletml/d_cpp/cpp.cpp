class C {
public:
	C() {}
	C(int a) : a_(a) {}

	int a() const { return a_; }
	void setA(int a) { a_ = a; }

	void f2(int, int) {}
	void f3(int, int, int) {}
	void f4(int, int, int, int) {}
	void f5(int, int, int, int, int) {}
	void f6(int, int, int, int, int, int) {}
	void f7(int, int, int, int, int, int, int) {}
	void f8(int, int, int, int, int, int, int, int) {}
	void f9(int, int, int, int, int, int, int, int, int) {}

	void over_load(int a) { a_ = a; }
	void over_load(char a) { a_ = a-48; }

	static int sf0() { return 6; }
	static int sf1(int) { return 7; }

private:
	int a_;
};

template <class T_> class TC {
public:
	T_ f() { return 5; }
	template <class T2_>
	T2_ tf() { return 6; }
};

class I {
public:
	int load_callback() {
		return callback();
	}
	int load_callback2(int x, int y) {
		return callback2(x, y);
	}
	virtual int callback() =0;
	virtual int callback2(int, int) =0;
};

#include "d_cpp_interface.h"

extern "C" {
	D_CPP_CLASS(C, D_C)
	D_CPP_NEW_0(C, D_C_new)
	D_CPP_NEW_1(C, D_C_new_1, int)
	D_CPP_DELETE(C, D_C_delete)

	D_CPP_METHOD_0(C, a, D_C_a, int)
	D_CPP_METHOD_1(C, setA, D_C_setA, void, int)

	D_CPP_METHOD_2(C, f2, D_C_f2, void, int, int)
	D_CPP_METHOD_3(C, f3, D_C_f3, void, int, int, int)
	D_CPP_METHOD_4(C, f4, D_C_f4, void, int, int, int, int)
	D_CPP_METHOD_5(C, f5, D_C_f5, void, int, int, int, int, int)
	D_CPP_METHOD_6(C, f6, D_C_f6, void, int, int, int, int, int, int)
	D_CPP_METHOD_7(C, f7, D_C_f7, void, int, int, int, int, int, int, int)
	D_CPP_METHOD_8(C, f8, D_C_f8, void, int, int, int, int, int, int, int, int)
	D_CPP_METHOD_9(C, f9, D_C_f9, void, int, int, int, int, int, int, int, int, int)

	D_CPP_METHOD_1(C, over_load, D_C_over_load_int, void, int)
	D_CPP_METHOD_1(C, over_load, D_C_over_load_char, void, char)

	D_CPP_CLASS(TC<int>, D_TC_int)
	D_CPP_NEW_0(TC<int>, D_TC_int_new)
	D_CPP_METHOD_0(TC<int>, f, D_TC_int_f, int)
	D_CPP_METHOD_0(TC<int>, tf<int>, D_TC_int_tf, int)
	D_CPP_DELETE(TC<int>, D_TC_int_delete)

	D_CPP_STATIC_METHOD_0(C, sf0, D_C_sf0, int)
	D_CPP_STATIC_METHOD_1(C, sf1, D_C_sf1, int, int)

}

// inherit

D_CPP_BASE_CLASS_OPEN(I, D_I)
D_CPP_VIRTUAL_METHOD_0(D_I, callback, int)
D_CPP_VIRTUAL_METHOD_2(D_I, callback2, int, int, int)
D_CPP_BASE_CLASS_CLOSE()

extern "C" {
	D_CPP_CLASS(D_I, D_I)
	D_CPP_NEW_0(D_I, D_I_new)
	D_CPP_METHOD_0(D_I, load_callback, D_I_load_callback, int)
	D_CPP_METHOD_2(D_I, load_callback2, D_I_load_callback2, int, int, int)
	D_CPP_VIRTUAL_METHOD_SETTER_0(D_I, callback, D_I_setCallbackFunc, int)
	D_CPP_VIRTUAL_METHOD_SETTER_2(D_I, callback2, D_I_setCallback2Func, int, int, int)
	D_CPP_DELETE(D_I, D_I_delete)
}

