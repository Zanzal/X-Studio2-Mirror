#pragma once

// Macro: Creates a wide char equivilent of an input string (from MSDN)
#define WIDEN2(x)                L ## x
#define WIDEN(x)                 WIDEN2(x)

#define STRINGIZE(n)             #n

// Macro: Wide character equivilent of __FILE__
#define __WFILE__                WIDEN(__FILE__)

// Macro: Wide character equivilent of __FUNCTION__
#define __WFUNCTION__            WIDEN(__FUNCTION__)

// Macro: Assembles wide character description of code location
#define HERE                     GuiString(L"%s() on line %d", __WFUNCTION__, __LINE__)
//#define HERE   __WFUNCTION__ L" on line " STRINGIZE(__LINE__)
//#define HERE   WIDEN(__FUNCTION__ " on line " STRINGIZE(__LINE__))

// Macro: Provides default move constructor and assignment (Not functional)
#define DEFAULT_MOVE(type)       // MS BUG:800114:     type::type(type&&) = default;  type& operator=(type&&) = default;

// Macro: Provides default copy constructor and assignment
#define DEFAULT_COPY(type)       type(const type&) = default;  type& operator=(const type&) = default;


// Macro: Prevents a class from being moved
#define NO_MOVE(type)            type(type&&) = delete;  NO_MOVE_ASSIGN(type)

// Macro: Prevents a class from being copied
#define NO_COPY(type)            type(const type&) = delete;  NO_COPY_ASSIGN(type)

// Macro: Disables move assignment operator
#define NO_MOVE_ASSIGN(type)     type& operator=(type&&) = delete;

// Macro: Disables copy assignment operator
#define NO_COPY_ASSIGN(type)     type& operator=(const type&) = delete;


// Macro: Declares a read-only property 
#define PROPERTY_GET(type,name,fget)   __declspec(property(get=fget)) type name

// Macro: Declares a read-write property 
#define PROPERTY_GET_SET(type,name,fget,fset)   __declspec(property(get=fget,put=fset)) type name

// Macro: Throws ArgumentNullException if argument is null
#define REQUIRED(arg)  { if ((arg) == nullptr) throw ArgumentNullException(HERE, WIDEN(#arg)); }

// Macro: Shorthand for console end-of-line manipulator
//#define ENDL      Cons::Endl

// Macro: ClassWizard fix that enables classes in namespaces
#define NAMESPACE_BEGIN(n)     namespace n {
#define NAMESPACE_END(n)       }  using namespace n;

// Macro: ClassWizard fix that enables classes in namespaces
#define NAMESPACE_BEGIN2(n1,n2)     namespace n1 {  namespace n2 {
#define NAMESPACE_END2(n1,n2)       } }  using namespace n1::n2;

// Macro: Forward declarations nested in namespaces
#define FORWARD_DECLARATION(ns,decl)  namespace ns { decl }
#define FORWARD_DECLARATION2(outer, inner,decl)  namespace outer { namespace inner { decl; } }


