// C:\mingw/lib/gcc/mingw32/4.6.1/include/c++/string
//lint -e10    Expecting '}'     //  I have NEVER understood where this comes from?!?!
//lint -e525   Negative indentation from line ...
//lint -e539   Did not expect positive indentation from line ...
//lint -e755   global macro not referenced
//lint -e534   Ignoring return value of function

//lint -esym(767, _WIN32_WINNT)

//lint -e818   Pointer parameter could be declared as pointing to const
//lint -e840   Use of nul character in a string literal
//lint -e845   The right argument to operator '||' is certain to be 0 
//lint -e1776  Converting a string literal to char * is not const safe (initialization)
//lint -e1786  Implicit conversion to Boolean (initialization) (long to bool)

//lint -e713   Loss of precision (arg. no. 2) (unsigned int to int)
//lint -e732   Loss of sign (initialization) (int to unsigned int)

//lint -e1066  Symbol declared as "C" conflicts with header file

//  more meaningless "errors" from C++ system headers
//lint -e18  Symbol 'std::__is_void<void>::__value' redeclared (enum/enum) conflicts with
//lint -e37  Value of enumerator 'std::__is_void<<1>>::__value' inconsistent (conflicts with
//lint -e31  Redefinition of symbol '_ctype' compare with line 99, file \mingw\include\wctype.h
//lint -e1087  Previous declaration of '__gnu_cxx::__is_signed' is incompatible with something else

//lint -e1704  Constructor has private access specification

//lint -esym(756, winproc_table_t)

