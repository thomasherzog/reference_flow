# Bank0: top-left, pins facing down
set bank0X $floor_leftX
set bankY [expr $floor_topY - $RamSize512x32_H]
placeInstance $bank0_sram0 $bank0X $bankY R0

# Bank1: top-right, pins facing down
set bank1X [expr $floor_rightX - $RamSize512x32_W]
placeInstance $bank1_sram0 $bank1X $bankY R0

utl::report "SRAM macro box: width ${RamSize512x32_W} height ${RamSize512x32_H}"
utl::report "SRAM bank0 bbox: ($bank0X, $bankY) - ([expr {$bank0X + $RamSize512x32_W}], [expr {$bankY + $RamSize512x32_H}]) R0"
utl::report "SRAM bank1 bbox: ($bank1X, $bankY) - ([expr {$bank1X + $RamSize512x32_W}], [expr {$bankY + $RamSize512x32_H}]) R0"
utl::report "SRAM horizontal gaps to core boundary: left [expr {$bank0X - $core_leftX}] between [expr {$bank1X - ($bank0X + $RamSize512x32_W)}] right [expr {$core_rightX - ($bank1X + $RamSize512x32_W)}]"
utl::report "SRAM vertical gap to core boundary: top [expr {$core_topY - ($bankY + $RamSize512x32_H)}]"
utl::report "SRAM row-cut halo: x $sramHaloX y $sramHaloY"

# ============
# SOCC ON CROC
# ============


# Glyph RAM 1
set socconcroc_gr_x $floor_leftX
set socconcroc_gr_y [expr $floor_bottomY + 2 * $sramHaloY + $RamSize512x32_H ]
placeInstance $socconcroc_sram_glyph0 $socconcroc_gr_x $socconcroc_gr_y R0

# Glyph RAM 2
set socconcroc_gr_x [expr $floor_leftX + + 2 * $sramHaloX + $RamSize512x32_W]
set socconcroc_gr_y [expr $floor_bottomY + 2 * $sramHaloY + $RamSize512x32_H ]
placeInstance $socconcroc_sram_glyph1 $socconcroc_gr_x $socconcroc_gr_y R0

# Glyph RAM 3
set socconcroc_gr_x $floor_leftX
set socconcroc_gr_y $floor_bottomY
placeInstance $socconcroc_sram_glyph2 $socconcroc_gr_x $socconcroc_gr_y R0

# Glyph RAM 4
set socconcroc_gr_x [expr $floor_leftX + + 2 * $sramHaloX + $RamSize512x32_W]
set socconcroc_gr_y $floor_bottomY
placeInstance $socconcroc_sram_glyph3 $socconcroc_gr_x $socconcroc_gr_y R0




# Text RAM 1
set socconcroc_tr_x [expr $floor_rightX - 3*$RamSize512x32_H - 4*$sramHaloX]
set socconcroc_tr_y [expr $floor_bottomY + $RamSize512x32_W + 2*$sramHaloY ]
placeInstance $socconcroc_sram_text0 $socconcroc_tr_x $socconcroc_tr_y R0


# Text RAM 2
set socconcroc_tr_x [expr $floor_rightX - 3*$RamSize512x32_H - 4*$sramHaloX]
set socconcroc_tr_y $floor_bottomY
placeInstance $socconcroc_sram_text1 $socconcroc_tr_x $socconcroc_tr_y R90

# Text RAM 3
set socconcroc_tr_x [expr $floor_rightX - 2*$RamSize512x32_H - 2*$sramHaloX]
set socconcroc_tr_y $floor_bottomY
placeInstance $socconcroc_sram_text2 $socconcroc_tr_x $socconcroc_tr_y R90

# Text RAM 4
set socconcroc_tr_x [expr $floor_rightX - $RamSize512x32_H]
set socconcroc_tr_y $floor_bottomY
placeInstance $socconcroc_sram_text3 $socconcroc_tr_x $socconcroc_tr_y R90


