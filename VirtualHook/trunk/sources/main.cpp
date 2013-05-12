#include <Windows.h>
#include <stdio.h>
#include <string>

class Base
{
public:
	virtual HRESULT WINAPI method( std::string const secret )
	{
		puts( "Base::method is processing the secret string!" );
		return 0;
	}
};

class Derived : public Base
{
public:
	HRESULT WINAPI method( std::string const secret )
	{
		puts( "Derived::method is processing the secret string!" );
		return 0;
	}
};

void * buff[32]; //holds original methods to maintain functionality in the hook

HRESULT WINAPI method_hook(Base * b, std::string const secret) //implicit 'this' pointer
{
	printf( "%s%s\n", "function redirected!\n\tMessage: ", secret.c_str() );
	((HRESULT (WINAPI *)( Base *b, std::string const ))buff[0])( b, secret ); // cast to proper function pointer and call in 1 line

	return 0;
}

void hookAPI( Base * bp, unsigned offset)
{
	//the first member of any base class or anything inhereting a base class
	//with virtual methods is a pointer to the virtual table
	DWORD oldprotection;
	DWORD placeholder;
	void * hook_ptr = &method_hook;
	void ** vtable = *(void***)bp;
	buff[ offset ] = vtable[ offset ]; //save the original for functionality
	VirtualProtect( vtable + offset , sizeof(void*), PAGE_READWRITE, &oldprotection );
	vtable[offset] = hook_ptr;
	VirtualProtect( vtable + offset , sizeof(void*), oldprotection, &placeholder );

}

int main( int const argc, char const * const * argv )
{
	Base * b = new Base;
	Derived * d = new Derived;
	char const * secret = "hidden data";
	hookAPI( b, 0 );
	b->method( secret );
	d->method( secret );

	delete b;
	delete d;

	return EXIT_SUCCESS;
}