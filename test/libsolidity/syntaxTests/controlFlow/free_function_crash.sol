contract C1 {
  function f() external {}
  function f(int) external {}
  function f2() external {}
}
function f() {}
contract C {
  function f() external {}
}
contract C2 is C0 {}
// ----
// Warning 2519: (16-40): This declaration shadows an existing declaration.
// Warning 2519: (43-70): This declaration shadows an existing declaration.
// Warning 2519: (132-156): This declaration shadows an existing declaration.
// DeclarationError 7920: (174-176): Identifier not found or not unique.
