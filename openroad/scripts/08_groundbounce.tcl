source scripts/startup.tcl
load_checkpoint 04_${proj_name}.routed

read_spef out/croc.spef
set_power_activity -input -activity 0.33
set_power_activity -input_port rst_ni -activity 0

report_power -corner tt

analyze_power_grid -vsrc src/Vsrc_croc_vss.loc -net VSS -corner tt
set_pdnsim_net_voltage -net VSS -voltage 0
