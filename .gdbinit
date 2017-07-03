target remote localhost:3333
mon soft_reset_halt
mon reset init

def reset
    mon soft_reset_halt
    mon reset init
    end
