#ifndef BTCLITE_TRANSACTION_SCRIPT_WITNESS_H
#define BTCLITE_TRANSACTION_SCRIPT_WITNESS_H

#include <string>
#include <vector>
#include <utility>

class ScriptWitness {
public:
	ScriptWitness() {}
	
	ScriptWitness(const ScriptWitness& witness)
		: stack_(witness.stack_) {}
	ScriptWitness(ScriptWitness&& witness) noexcept
		: stack_(std::move(witness.stack_)) {}
	
	explicit ScriptWitness(const std::vector<std::vector<uint8_t> >& stack)
		: stack_(stack) {}
	explicit ScriptWitness(std::vector<std::vector<uint8_t> >&& stack) noexcept
		: stack_(std::move(stack)) {}
	
	//-------------------------------------------------------------------------
	bool IsNull() const
	{
		return stack_.empty();
	}

    void Clear()
	{
		stack_.clear();
		stack_.shrink_to_fit();
	}

    std::string ToString() const;
	
	//-------------------------------------------------------------------------
	bool operator==(const ScriptWitness& b) const
	{
		return stack_ == b.stack_;
	}
	bool operator!=(const ScriptWitness& b) const
	{
		return !(*this == b);
	}
	
	ScriptWitness& operator=(const ScriptWitness& b)
	{
		stack_ = b.stack_;
		return *this;
	}
	ScriptWitness& operator=(ScriptWitness&& b)
	{
		if (this != &b)
			stack_ = std::move(b.stack_);
		return *this;
	}
	
	//-------------------------------------------------------------------------
	const std::vector<std::vector<uint8_t> >& stack() const
	{
		return stack_;
	}
	void set_stack(const std::vector<std::vector<uint8_t> >& stack)
	{
		stack_ = stack;
	}
	void set_stack(std::vector<std::vector<uint8_t> >&& stack)
	{
		stack_ = std::move(stack);
	}
	
private:
	std::vector<std::vector<uint8_t> > stack_;
};

#endif // BTCLITE_TRANSACTION_SCRIPT_WITNESS_H
