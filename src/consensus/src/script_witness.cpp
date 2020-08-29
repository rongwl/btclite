#include "script_witness.h"


namespace btclite {
namespace consensus {


ScriptWitness::ScriptWitness(const ScriptWitness& witness)
    : stack_(witness.stack_)
{
}

ScriptWitness::ScriptWitness(ScriptWitness&& witness) noexcept
    : stack_(std::move(witness.stack_))
{
}

ScriptWitness::ScriptWitness(const std::vector<std::vector<uint8_t> >& stack)
    : stack_(stack)
{
}

ScriptWitness::ScriptWitness(std::vector<std::vector<uint8_t> >&& stack) noexcept
    : stack_(std::move(stack)) 
{
}

bool ScriptWitness::IsNull() const
{
    return stack_.empty();
}

void ScriptWitness::Clear()
{
    stack_.clear();
    stack_.shrink_to_fit();
}

std::string ScriptWitness::ToString() const
{
    return "";
}

bool ScriptWitness::operator==(const ScriptWitness& b) const
{
    return stack_ == b.stack_;
}

bool ScriptWitness::operator!=(const ScriptWitness& b) const
{
    return !(*this == b);
}

ScriptWitness& ScriptWitness::operator=(const ScriptWitness& b)
{
    stack_ = b.stack_;
    return *this;
}

ScriptWitness& ScriptWitness::operator=(ScriptWitness&& b)
{
    if (this != &b)
        stack_ = std::move(b.stack_);
    return *this;
}

const std::vector<std::vector<uint8_t> >& ScriptWitness::stack() const
{
    return stack_;
}

void ScriptWitness::set_stack(const std::vector<std::vector<uint8_t> >& stack)
{
    stack_ = stack;
}

void ScriptWitness::set_stack(std::vector<std::vector<uint8_t> >&& stack)
{
    stack_ = std::move(stack);
}

} // namespace consensus
} // namespace btclite
