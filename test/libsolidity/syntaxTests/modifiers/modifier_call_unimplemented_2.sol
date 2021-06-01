// SPDX-License-Identifier: UNLICENSED
pragma solidity >= 0.5.0;

abstract contract A {
    modifier m() virtual;
}
contract B is A {
    modifier m() virtual override { _; }
}
contract C is B {
    function f() A.m public {}
}
// ----
// TypeError 9113: (212-215): Cannot call unimplemented modifier.
