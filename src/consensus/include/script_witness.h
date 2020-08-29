#ifndef BTCLITE_CONSENSUS_SCRIPT_WITNESS_H
#define BTCLITE_CONSENSUS_SCRIPT_WITNESS_H

#include <string>
#include <vector>
#include <utility>


namespace btclite {
namespace consensus {

class ScriptWitness {
public:
    ScriptWitness() = default;
    
    ScriptWitness(const ScriptWitness& witness);
    ScriptWitness(ScriptWitness&& witness) noexcept;
    
    explicit ScriptWitness(const std::vector<std::vector<uint8_t> >& stack);
    explicit ScriptWitness(std::vector<std::vector<uint8_t> >&& stack) noexcept;
    
    //-------------------------------------------------------------------------
    bool IsNull() const;
    void Clear();
    std::string ToString() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const ScriptWitness& b) const;
    bool operator!=(const ScriptWitness& b) const;
    
    ScriptWitness& operator=(const ScriptWitness& b);
    ScriptWitness& operator=(ScriptWitness&& b);
    
    //-------------------------------------------------------------------------
    const std::vector<std::vector<uint8_t> >& stack() const;
    void set_stack(const std::vector<std::vector<uint8_t> >& stack);
    void set_stack(std::vector<std::vector<uint8_t> >&& stack);
    
private:
    std::vector<std::vector<uint8_t> > stack_;
};

} // namespace consensus
} // namespace btclite

#endif // BTCLITE_CONSENSUS_SCRIPT_WITNESS_H
