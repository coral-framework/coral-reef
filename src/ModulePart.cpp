/*
 * reef - Remoting Framework
 * See copyright notice in LICENSE.md
 */

#include "reef_Base.h"
#include "ModuleInstaller.h"
#include <co/Coral.h>
#include <co/ISystem.h>
#include <co/IModuleManager.h>

#include "Message.pb.h"

namespace reef {

/*!
	The reef module's co.IModulePart.
 */
class ModulePart : public reef::reef_Base
{
public:
    ModulePart()
	{
		// empty
	}

	virtual ~ModulePart()
	{
		// empty
	}

	void initialize( co::IModule* module )
	{
        GOOGLE_PROTOBUF_VERIFY_VERSION;
		reef::ModuleInstaller::instance().install();
	}

	void integrate( co::IModule* )
	{
		// empty
	}

	void integratePresentation( co::IModule* )
	{
		// empty
	}

	void disintegrate( co::IModule* )
	{
        google::protobuf::ShutdownProtobufLibrary();
	}

	void dispose( co::IModule* )
	{
		reef::ModuleInstaller::instance().uninstall();
	}
};

CORAL_EXPORT_MODULE_PART( ModulePart );
CORAL_EXPORT_COMPONENT( ModulePart, reef );

} // namespace reef
