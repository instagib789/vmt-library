### VMT Library

A Windows x64 library that recovers [RTTI](https://en.wikipedia.org/wiki/Run-time_type_information) and searches modules
for virtual method tables based on their type name. It also provides an easy
interface for duplicating a table, swapping out function indices, and hooking class instances by swapping their virtual
table pointer. I recommend using the [ClassInformer](https://sourceforge.net/projects/classinformer/) IDA Pro plugin to
find the type names and function indices you would like to hook.

**Examples**

```c++
// Create an example class that uses virtual inheritance.
Derived* p_derived = new Derived();

// Call a member function normally.
p_derived->Button();

// Search RTTI to find the Derived class's VMT and determine it's size.
auto [table_address, table_size] = vmt::Find("module.exe", ".?AVDerived@@");

// Instantiating the Hook class will create a copy of the table.
vmt::Hook table_hook(table_address, table_size);

// Replace the function pointer at index 1 in our copy table.
// Note: Make sure the hook function's first parameter is the 'this' pointer.
table_hook.SwapIndex(1, ButtonHook);

// Hook the class instance we created by swapping its table pointer to our copy.
table_hook.HookInstance(p_derived);

// Now when this instance calls Button, instead ButtonHook will be called.
p_derived->Button();
```

**Requires**

- C++ 20 or later.
- Windows x64
- [Module Library](https://github.com/instagib789/module-library).