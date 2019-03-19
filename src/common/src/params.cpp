#include "common/include/params.h"
#include "script.h"

Common::Params::Params(Environment env)
	: consensus_(Consensus::Params(env))
{

}

