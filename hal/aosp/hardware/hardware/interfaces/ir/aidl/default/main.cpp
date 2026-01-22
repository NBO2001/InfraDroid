#include "ConsumerIr.h" // Inclui nossa classe

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

using aidl::android::hardware::ir::ConsumerIr;

int main() {
    LOG(INFO) << "Iniciando IR HAL Service...";

    ABinderProcess_setThreadPoolMaxThreadCount(0);

    // Cria a instância da nossa lógica
    std::shared_ptr<ConsumerIr> ir = ndk::SharedRefBase::make<ConsumerIr>();

    // Registra o serviço com o nome padrão
    const std::string instance = std::string(ConsumerIr::descriptor) + "/default";
    binder_status_t status = AServiceManager_addService(ir->asBinder().get(), instance.c_str());

    CHECK(status == STATUS_OK);

    LOG(INFO) << "IR HAL Service registrado com sucesso!";

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE; // Não deve chegar aqui
}
