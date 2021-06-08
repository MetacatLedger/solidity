/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
// SPDX-License-Identifier: GPL-3.0
/**
 * Data flow graph and stack layout structures used during code generation.
 */

#pragma once

#include <libyul/AST.h>
#include <libyul/AsmAnalysisInfo.h>
#include <libyul/backends/evm/EVMDialect.h>

#include <libyul/optimiser/ASTWalker.h>
#include <libyul/Scope.h>

#include <vector>

namespace solidity::yul
{

/// The label pushed as return label before a function call, i.e. the label the call is supposed to return to.
struct FunctionCallReturnLabelSlot
{
	std::reference_wrapper<yul::FunctionCall const> call;
	bool operator==(FunctionCallReturnLabelSlot const& _rhs) const { return &call.get() == &_rhs.call.get(); }
	bool operator<(FunctionCallReturnLabelSlot const& _rhs) const { return &call.get() < &_rhs.call.get(); }
};
/// The return label of a function while generating the code of the function body.
struct FunctionReturnLabelSlot
{
	bool operator==(FunctionReturnLabelSlot const&) const { return true; }
	bool operator<(FunctionReturnLabelSlot const&) const { return false; }
};
/// A slot containing the current value of a particular variable.
struct VariableSlot
{
	std::reference_wrapper<Scope::Variable const> variable;
	std::shared_ptr<DebugData const> debugData{};
	bool operator==(VariableSlot const& _rhs) const { return &variable.get() == &_rhs.variable.get(); }
	bool operator<(VariableSlot const& _rhs) const { return &variable.get() < &_rhs.variable.get(); }
};
/// A slot containing a literal value.
struct LiteralSlot
{
	u256 value;
	std::shared_ptr<DebugData const> debugData{};
	bool operator==(LiteralSlot const& _rhs) const { return value == _rhs.value; }
	bool operator<(LiteralSlot const& _rhs) const { return value < _rhs.value; }
};
/// A slot containing the idx-th return value of a previous call.
struct TemporarySlot
{
	/// The call that returned this slot.
	std::reference_wrapper<yul::FunctionCall const> call;
	/// Specifies to which of the values returned by the call this slot refers.
	/// index == 0 refers to the slot deepest in the stack after the call.
	size_t index = 0;
	bool operator==(TemporarySlot const& _rhs) const { return &call.get() == &_rhs.call.get() && index == _rhs.index; }
	bool operator<(TemporarySlot const& _rhs) const { return std::make_pair(&call.get(), index) < std::make_pair(&_rhs.call.get(), _rhs.index); }
};
/// A slot containing an arbitrary value that is always eventually popped and never used.
/// Used to maintain stack balance on control flow joins.
struct JunkSlot
{
	bool operator==(JunkSlot const&) const { return true; }
	bool operator<(JunkSlot const&) const { return false; }
};
using StackSlot = std::variant<FunctionCallReturnLabelSlot, FunctionReturnLabelSlot, VariableSlot, LiteralSlot, TemporarySlot, JunkSlot>;
/// The stack top is usually the last element of the vector.
using Stack = std::vector<StackSlot>;

/// Data flow graph consisting of ``DFG::BasicBlock``s connected by control flow.
struct DFG
{
	explicit DFG() {}
	DFG(DFG const&) = delete;
	DFG(DFG&&) = delete;
	DFG& operator=(DFG const&) = delete;
	DFG& operator=(DFG&&) = delete;

	struct BuiltinCall
	{
		std::shared_ptr<DebugData const> debugData;
		std::reference_wrapper<BuiltinFunctionForEVM const> builtin;
		std::reference_wrapper<yul::FunctionCall const> functionCall;
		/// Number of proper arguments with a position on the stack, excluding literal arguments.
		size_t arguments = 0;
	};
	struct FunctionCall
	{
		std::shared_ptr<DebugData const> debugData;
		std::reference_wrapper<Scope::Function const> function;
		std::reference_wrapper<yul::FunctionCall const> functionCall;
	};
	struct Assignment
	{
		std::shared_ptr<DebugData const> debugData;
		/// The variables being assigned to also occur as ``output`` in the ``Operation`` containing
		/// the assignment, but are also stored here for convenience.
		std::vector<VariableSlot> variables;
	};

	struct Operation
	{
		/// Stack slots this operation expects at the top of the stack and consumes.
		Stack input;
		/// Stack slots this operation leaves on the stack as output.
		Stack output;
		std::variant<FunctionCall, BuiltinCall, Assignment> operation;
	};

	struct FunctionInfo;
	/// A basic control flow block containing ``Operation``s acting on the stack.
	/// Maintains a list of entry blocks and a typed exit.
	struct BasicBlock
	{
		std::vector<BasicBlock const*> entries;
		std::vector<Operation> operations;
		struct MainExit {};
		struct ConditionalJump
		{
			StackSlot condition;
			BasicBlock* nonZero = nullptr;
			BasicBlock* zero = nullptr;
		};
		struct Jump
		{
			BasicBlock* target = nullptr;
			/// The only backwards jumps are jumps from loop post to loop condition.
			bool backwards = false;
		};
		struct FunctionReturn { DFG::FunctionInfo* info = nullptr; };
		struct Terminated {};
		std::variant<MainExit, Jump, ConditionalJump, FunctionReturn, Terminated> exit = MainExit{};
	};

	struct FunctionInfo
	{
		std::shared_ptr<DebugData const> debugData;
		Scope::Function const& function;
		BasicBlock* entry = nullptr;
		std::vector<VariableSlot> parameters;
		std::vector<VariableSlot> returnVariables;
	};

	/// The main entry point, i.e. the start of the outermost Yul block.
	BasicBlock* entry = nullptr;
	/// Subgraphs for functions.
	std::map<Scope::Function const*, FunctionInfo> functionInfo;
	/// List of functions in order of declaration.
	std::list<Scope::Function const*> functions;

	/// Container for blocks for explicit ownership.
	std::list<BasicBlock> blocks;
	/// Container for creates variables for explicit ownership.
	std::list<Scope::Variable> ghostVariables;
	/// Container for creates calls for explicit ownership.
	std::list<yul::FunctionCall> ghostCalls;

	BasicBlock& makeBlock()
	{
		return blocks.emplace_back(BasicBlock{});
	}
};

}