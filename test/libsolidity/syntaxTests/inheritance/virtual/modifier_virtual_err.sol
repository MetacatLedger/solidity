library test {
    modifier m virtual;
    function f() m public {
    }
}
// ----
// TypeError 3275: (19-38): Modifiers in a library cannot be virtual.
// TypeError 9113: (56-57): Cannot call unimplemented modifier.
