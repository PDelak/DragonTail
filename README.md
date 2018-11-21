## CoGeCs - Low level language for JIT

### Transformations

* `if statement`
if statement is transformed in the following way

from 
~~~~~~~~~~~~~~~~~~~~~~~~none
var x;
if (x) {
	var y;
}
~~~~~~~~~~~~~~~~~~~~~~~~
to 
~~~~~~~~~~~~~~~~~~~~~~~~none
var x;
var temp__0;
temp__0 = x;
if (!temp__0) goto label__1;
var y;
label__1:
~~~~~~~~~~~~~~~~~~~~~~~~

* `while statement`
while statement is transformed in the following way

from 
~~~~~~~~~~~~~~~~~~~~~~~~none
var x;
while (x) {
	var y;
}
~~~~~~~~~~~~~~~~~~~~~~~~
to 
~~~~~~~~~~~~~~~~~~~~~~~~none
var temp__0;
label__1:
temp__0=x;
if (!temp__0) goto label__2;
var y;
goto label__1;
label__2:
~~~~~~~~~~~~~~~~~~~~~~~~

### Function support 

* `store and load builtin functions for stack operations`
* `pointer type`

#### Stack frame
~~~~~~~~~~~~~~~~~~~~~~~~none
+---------------------+
| Parameter n         |
+---------------------+
| Paremeter n-1       |
+---------------------+
| Parameter 1         |
+---------------------+
| Return value address|
+---------------------+
| Return address      |
+---------------------+
~~~~~~~~~~~~~~~~~~~~~~~~