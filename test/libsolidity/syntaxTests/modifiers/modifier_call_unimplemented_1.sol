// SPDX-License-Identifier: UNLICENSED
pragma solidity >= 0.5.0;

contract A {
    modifier m() virtual { _; }
}
abstract contract B {
    modifier m() virtual;
}
contract C is A, B {
    modifier m() override(A, B) { _; }
    function f() B.m public {}
}
// ----
// TypeError 9113: (240-243): Cannot call unimplemented modifier.
