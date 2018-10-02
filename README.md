CoGeCs - Low level language for JIT


### Function support 

* `store and load builtin functions for stack operations`
* `pointer type`

#### Stack frame
~~~~~~~~~~~~~~~~~~~~~~~~none
+---------------------+
| Return value address|
+---------------------+
| Return address      |
+---------------------+
| Parameter n         |
+---------------------+
| Paremeter n-1       |
+---------------------+
| Parameter 1         |
+---------------------+
~~~~~~~~~~~~~~~~~~~~~~~~