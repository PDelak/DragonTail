CoGeCs - Low level language for JIT

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
var temp__0;
temp__0 = x;
if (!temp__0) goto label__1;
var y;
label__1:
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