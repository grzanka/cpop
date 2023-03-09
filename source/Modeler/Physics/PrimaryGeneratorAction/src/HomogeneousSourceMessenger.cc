#include "UniformSourceMessenger.hh"
#include "UniformSource.hh"

namespace cpop {

UniformSourceMessenger::UniformSourceMessenger(UniformSource *source)
    : MessengerBase(),
      source_(source),
      source_messenger_(std::make_unique<SourceMessenger>(source))
{
}

void UniformSourceMessenger::BuildCommands(G4String base)
{
    source_messenger_->BuildCommands(base);

    G4String cmd_base = base + "/totalParticle";
    total_particle_cmd_ = std::make_unique<G4UIcmdWithAnInteger>(cmd_base, this);
    total_particle_cmd_->SetGuidance("Set number of particles to be generated from this source");
    total_particle_cmd_->SetParameterName("TotalParticle", false);
    total_particle_cmd_->AvailableForStates(G4State_PreInit, G4State_Idle);
}

void UniformSourceMessenger::SetNewValue(G4UIcommand *command, G4String newValue)
{
    source_messenger_->SetNewValue(command, newValue);

    if (command == total_particle_cmd_.get()) {
        source_->setTotal_particle(total_particle_cmd_->GetNewIntValue(newValue));
    }
}

}
