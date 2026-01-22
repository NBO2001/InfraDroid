#include "ConsumerIr.h"

#include <android-base/logging.h> // Para LOG(INFO)
#include <fstream>                // Para escrever no arquivo
#include <iostream>

// O CAMINHO EXATO QUE CRIAMOS NO KERNEL
const std::string kIrDriverPath = "/sys/class/infrared/ir_module/transmit";

namespace aidl {
namespace android {
namespace hardware {
namespace ir {

// 1. Diz ao Android que aceitamos qualquer frequência comum (30k a 60k)
::ndk::ScopedAStatus ConsumerIr::getCarrierFreqs(std::vector<ConsumerIrFreqRange>* _aidl_return) {
    ConsumerIrFreqRange range;
    range.minHz = 30000;
    range.maxHz = 60000;
    _aidl_return->push_back(range);
    return ::ndk::ScopedAStatus::ok();
}

// 2. Recebe o comando e envia para o Driver
::ndk::ScopedAStatus ConsumerIr::transmit(int32_t carrierFreqHz, const std::vector<int32_t>& pattern) {
    // Conversão: O Android manda em Hz (38000), o Driver quer kHz (38)
    int freqKHz = carrierFreqHz / 1000;

    // Monta a string: "SEND 38 "
    std::string command = "SEND " + std::to_string(freqKHz);

    // Adiciona os tempos: "SEND 38 9000 4500 550..."
    for (int32_t time : pattern) {
        command += " " + std::to_string(time);
    }

    // Log para debug (aparece no Logcat)
    LOG(INFO) << "IR HAL: Escrevendo no Driver: " << command.substr(0, 50) << "...";

    // Abre o arquivo do Kernel e escreve
    std::ofstream driverFile(kIrDriverPath);
    if (!driverFile.is_open()) {
        LOG(ERROR) << "IR HAL: Erro ao abrir " << kIrDriverPath << ". Verifique permissoes!";
        // Retorna erro genérico para o Android não travar
        return ::ndk::ScopedAStatus::fromServiceSpecificError(1);
    }

    driverFile << command;

    if (driverFile.bad()) {
        LOG(ERROR) << "IR HAL: Erro ao escrever dados.";
        return ::ndk::ScopedAStatus::fromServiceSpecificError(2);
    }

    driverFile.close();
    return ::ndk::ScopedAStatus::ok();
}

}  // namespace ir
}  // namespace hardware
}  // namespace android
}  // namespace aidl