## DragonTail - Low level language for JIT

DragonTail is an ambitious project to create low level language with implementation 
of most important state of the art optimizations.

Work is divided into few sections:
* `supporting all primitives (loops and pointer handling is sill ongoing work)`
* `support for functions`
* `support for compound types (structs)`
* `x86 code generation`
* `x86-64 code generation`
* `linear register allocation` 

### Building
* `MSVC`

In order to build using MSVC run Configure_MSVC.bat or Configure_MSVCx64.bat depending on platform then
~~~~~~~~~~~~~~~~~~~~~~~~none
cmake --build ./build-msvc
~~~~~~~~~~~~~~~~~~~~~~~~
* `MinGW`

In order to build using MinGW run Configure_Make.bat (You have to use 32 bit version of gcc. x86-64 is unsupported for now) 
~~~~~~~~~~~~~~~~~~~~~~~~none
cmake --build ./build-make
~~~~~~~~~~~~~~~~~~~~~~~~

* `Linux`

Run Configure_Make.sh
~~~~~~~~~~~~~~~~~~~~~~~~none
cmake --build ./build-make
~~~~~~~~~~~~~~~~~~~~~~~~
 
