#include <gtest/gtest.h>

#include "Node.h"
#include "Marshaller.h"
#include "Demarshaller.h"
#include <ClientProxy.h>
#include <Message.pb.h>
#include "Invoker.h"

#include <stubs/ISimpleTypes.h>
#include <stubs/IReferenceTypes.h>

#include <co/Coral.h>
#include <co/IPort.h>
#include <co/IField.h>
#include <co/IMethod.h>
#include <co/IObject.h>

#include <iostream>

namespace rpc {
    
TEST( ServerTests, invokerValueTypeTest )
{
}

TEST( ServerTests, invokerReceivesRefTypeTests )
{
}

TEST( ServerTests, nodeTest )
{
}
    
} // namespace rpc