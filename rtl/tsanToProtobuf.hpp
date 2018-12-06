#include "pb_report.pb.h"

PBReportType tsToPbReportType (const __tsan::ReportType& typ);

PBModuleArch tsToPbModuleArch (const __tsan::ModuleArch& typ);

PBReportLocationType tsToPbReportLocationType (const __tsan::ReportLocationType& typ);

PBAddressInfo* tsToPbAddressInfo (const __tsan::AddressInfo& tsan_obj);

PBSymbolizedStack* tsToPbSymbolizedStack (const __tsan::SymbolizedStack& tsan_obj);

PBReportStack* tsToPbReportStack (const __tsan::ReportStack& tsan_obj);

PBReportMopMutex* tsToPbReportMopMutex (const __tsan::ReportMopMutex& tsan_obj);

PBReportMop* tsToPbReportMop (const __tsan::ReportMop& tsan_obj);

PBDataInfo* tsToPbDataInfo (const __tsan::DataInfo& tsan_obj);

PBReportLocation* tsToPbReportLocation (const __tsan::ReportLocation& tsan_obj);

PBReportMutex* tsToPbReportMutex (const __tsan::ReportMutex& tsan_obj);

PBReportThread* tsToPbReportThread (const __tsan::ReportThread& tsan_obj);

PBReportDesc* tsToPbReportDesc (const __tsan::ReportDesc& tsan_obj);
