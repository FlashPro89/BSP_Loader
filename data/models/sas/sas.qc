$modelname "D:\GD\BSP_Loader\data\models\sas\sas.mdl"
$cd "\GD\BSP_Loader\data\models\sas\"
$cdtexture "\GD\BSP_Loader\data\models\sas\"
$cliptotextures

$scale 1.0

// 2 attachments
$attachment 0 "Bip01 R Hand" 10.855000 -0.416715 1.870680 
$attachment 1 "Bip01 L Hand" 10.855000 -0.416715 1.870680 

// 1 bone controllers
$controller mouth "Bone01" ZR 0 30

// 21 hit boxes
$hbox 3 "Bip01 Pelvis" -4.690000 -4.440000 -6.750000  4.000000 5.560000 6.750000
$hbox 6 "Bip01 L Thigh" 2.660000 -3.690000 -3.090000  18.160000 4.880000 3.310000
$hbox 6 "Bip01 L Calf" 0.380000 -3.970000 -2.840000  17.600000 4.000000 2.940000
$hbox 6 "Bip01 L Foot" 0.850000 -3.240000 -2.640000  5.730000 8.530000 3.170000
$hbox 7 "Bip01 R Thigh" 2.470000 -3.690000 -3.160000  18.129999 4.880000 3.380000
$hbox 7 "Bip01 R Calf" 0.310000 -3.970000 -2.840000  17.600000 3.940000 2.970000
$hbox 7 "Bip01 R Foot" 0.850000 -3.240000 -2.640000  5.730000 8.530000 3.170000
$hbox 3 "Bip01 Spine1" -3.250000 -5.500000 -5.500000  4.830000 5.500000 5.500000
$hbox 2 "Bip01 Spine2" -0.060000 -5.530000 -7.590000  8.000000 7.000000 7.590000
$hbox 2 "Bip01 Spine3" -2.250000 -6.810000 -6.310000  6.500000 5.090000 6.310000
$hbox 2 "Bip01 Neck" -3.110000 -1.500000 -3.000000  2.050000 3.500000 3.000000
$hbox 1 "Bip01 Head" -0.330000 -4.500000 -2.500000  8.000000 4.500000 2.500000
$hbox 4 "Bip01 L Clavicle" 0.940000 -2.880000 -4.130000  5.440000 4.280000 3.500000
$hbox 4 "Bip01 L UpperArm" -2.160000 -2.340000 -2.560000  11.560000 3.410000 2.380000
$hbox 4 "Bip01 L Forearm" 0.590000 -1.810000 -2.190000  10.750000 2.840000 2.410000
$hbox 4 "Bip01 L Hand" -0.132201 -1.903020 -2.238530  4.862900 2.078240 2.369790
$hbox 5 "Bip01 R Clavicle" 1.380000 -2.840000 -3.560000  5.690000 4.310000 4.310000
$hbox 5 "Bip01 R UpperArm" -1.880000 -2.470000 -2.160000  11.810000 3.280000 2.810000
$hbox 5 "Bip01 R Forearm" 0.440000 -1.970000 -2.340000  10.590000 2.720000 2.220000
$hbox 5 "Bip01 R Hand" -0.132201 -1.903020 -2.238530  4.862900 2.078240 2.369790
$hbox 8 "Bip01 L Hand" -16.100000 -6.500000 -19.000000  10.600000 1.500000 33.000000

$bodygroup studio
{
studio "SAS"
}

$bodygroup backpack
{
blank
studio "..\packs\defuse"
}

// 111 sequences
$sequence dummy "dummy" loop fps 24 
$sequence idle1 "idle1" loop fps 15 ACT_IDLE 1 
$sequence crouch_idle "crouch_idle" loop fps 10 ACT_CROUCHIDLE 1 
$sequence walk "walk" LX loop fps 30 ACT_WALK 1 
$sequence run "run" LX loop fps 60 ACT_RUN 1 
$sequence crouchrun "crouchrun" LX loop fps 30 ACT_CROUCH 1 
$sequence jump "jump" fps 30 ACT_HOP 1 
$sequence longjump "longjump" fps 30 ACT_LEAP 1 
$sequence swim "swim" loop fps 30 ACT_SWIM 1 
$sequence treadwater "treadwater" loop fps 24 ACT_HOVER 1 
$sequence crouch_aim_carbine "crouch_aim_carbine_blend1" "crouch_aim_carbine_blend2" "crouch_aim_carbine_blend3" "crouch_aim_carbine_blend4" "crouch_aim_carbine_blend5" "crouch_aim_carbine_blend6" "crouch_aim_carbine_blend7" "crouch_aim_carbine_blend8" "crouch_aim_carbine_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_carbine "crouch_shoot_carbine_blend1" "crouch_shoot_carbine_blend2" "crouch_shoot_carbine_blend3" "crouch_shoot_carbine_blend4" "crouch_shoot_carbine_blend5" "crouch_shoot_carbine_blend6" "crouch_shoot_carbine_blend7" "crouch_shoot_carbine_blend8" "crouch_shoot_carbine_blend9" fps 15 blend XR -90 90 { event 5001 0 "40" } 
$sequence crouch_reload_carbine "crouch_reload_carbine" fps 30 
$sequence ref_aim_carbine "ref_aim_carbine_blend1" "ref_aim_carbine_blend2" "ref_aim_carbine_blend3" "ref_aim_carbine_blend4" "ref_aim_carbine_blend5" "ref_aim_carbine_blend6" "ref_aim_carbine_blend7" "ref_aim_carbine_blend8" "ref_aim_carbine_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_carbine "ref_shoot_carbine_blend1" "ref_shoot_carbine_blend2" "ref_shoot_carbine_blend3" "ref_shoot_carbine_blend4" "ref_shoot_carbine_blend5" "ref_shoot_carbine_blend6" "ref_shoot_carbine_blend7" "ref_shoot_carbine_blend8" "ref_shoot_carbine_blend9" fps 15 blend XR -90 90 { event 5001 0 "40" } 
$sequence ref_reload_carbine "ref_reload_carbine" fps 30 
$sequence crouch_aim_onehanded "crouch_aim_onehanded_blend1" "crouch_aim_onehanded_blend2" "crouch_aim_onehanded_blend3" "crouch_aim_onehanded_blend4" "crouch_aim_onehanded_blend5" "crouch_aim_onehanded_blend6" "crouch_aim_onehanded_blend7" "crouch_aim_onehanded_blend8" "crouch_aim_onehanded_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_onehanded "crouch_shoot_onehanded_blend1" "crouch_shoot_onehanded_blend2" "crouch_shoot_onehanded_blend3" "crouch_shoot_onehanded_blend4" "crouch_shoot_onehanded_blend5" "crouch_shoot_onehanded_blend6" "crouch_shoot_onehanded_blend7" "crouch_shoot_onehanded_blend8" "crouch_shoot_onehanded_blend9" fps 15 blend XR -90 90 { event 5001 0 "10" } 
$sequence crouch_reload_onehanded "crouch_reload_onehanded" fps 30 
$sequence ref_aim_onehanded "ref_aim_onehanded_blend1" "ref_aim_onehanded_blend2" "ref_aim_onehanded_blend3" "ref_aim_onehanded_blend4" "ref_aim_onehanded_blend5" "ref_aim_onehanded_blend6" "ref_aim_onehanded_blend7" "ref_aim_onehanded_blend8" "ref_aim_onehanded_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_onehanded "ref_shoot_onehanded_blend1" "ref_shoot_onehanded_blend2" "ref_shoot_onehanded_blend3" "ref_shoot_onehanded_blend4" "ref_shoot_onehanded_blend5" "ref_shoot_onehanded_blend6" "ref_shoot_onehanded_blend7" "ref_shoot_onehanded_blend8" "ref_shoot_onehanded_blend9" fps 15 blend XR -90 90 { event 5001 0 "10" } 
$sequence ref_reload_onehanded "ref_reload_onehanded" fps 30 
$sequence crouch_aim_dualpistols "crouch_aim_dualpistols_blend1" "crouch_aim_dualpistols_blend2" "crouch_aim_dualpistols_blend3" "crouch_aim_dualpistols_blend4" "crouch_aim_dualpistols_blend5" "crouch_aim_dualpistols_blend6" "crouch_aim_dualpistols_blend7" "crouch_aim_dualpistols_blend8" "crouch_aim_dualpistols_blend9" loop fps 10 blend XR -90 90 
$sequence crouch_shoot_dualpistols "crouch_shoot_dualpistols_blend1" "crouch_shoot_dualpistols_blend2" "crouch_shoot_dualpistols_blend3" "crouch_shoot_dualpistols_blend4" "crouch_shoot_dualpistols_blend5" "crouch_shoot_dualpistols_blend6" "crouch_shoot_dualpistols_blend7" "crouch_shoot_dualpistols_blend8" "crouch_shoot_dualpistols_blend9" fps 30 blend XR -90 90 { event 5011 0 "10" } 
$sequence crouch_shoot2_dualpistols "crouch_shoot2_dualpistols_blend1" "crouch_shoot2_dualpistols_blend2" "crouch_shoot2_dualpistols_blend3" "crouch_shoot2_dualpistols_blend4" "crouch_shoot2_dualpistols_blend5" "crouch_shoot2_dualpistols_blend6" "crouch_shoot2_dualpistols_blend7" "crouch_shoot2_dualpistols_blend8" "crouch_shoot2_dualpistols_blend9" fps 30 blend XR -90 90 { event 5001 0 "10" } 
$sequence crouch_reload_dualpistols "crouch_reload_dualpistols" fps 30 
$sequence ref_aim_dualpistols "ref_aim_dualpistols_blend1" "ref_aim_dualpistols_blend2" "ref_aim_dualpistols_blend3" "ref_aim_dualpistols_blend4" "ref_aim_dualpistols_blend5" "ref_aim_dualpistols_blend6" "ref_aim_dualpistols_blend7" "ref_aim_dualpistols_blend8" "ref_aim_dualpistols_blend9" loop fps 10 blend XR -90 90 
$sequence ref_shoot_dualpistols "ref_shoot_dualpistols_blend1" "ref_shoot_dualpistols_blend2" "ref_shoot_dualpistols_blend3" "ref_shoot_dualpistols_blend4" "ref_shoot_dualpistols_blend5" "ref_shoot_dualpistols_blend6" "ref_shoot_dualpistols_blend7" "ref_shoot_dualpistols_blend8" "ref_shoot_dualpistols_blend9" fps 30 blend XR -90 90 { event 5011 0 "10" } 
$sequence ref_shoot2_dualpistols "ref_shoot2_dualpistols_blend1" "ref_shoot2_dualpistols_blend2" "ref_shoot2_dualpistols_blend3" "ref_shoot2_dualpistols_blend4" "ref_shoot2_dualpistols_blend5" "ref_shoot2_dualpistols_blend6" "ref_shoot2_dualpistols_blend7" "ref_shoot2_dualpistols_blend8" "ref_shoot2_dualpistols_blend9" fps 30 blend XR -90 90 { event 5001 0 "10" } 
$sequence ref_reload_dualpistols "ref_reload_dualpistols" fps 30 
$sequence crouch_aim_rifle "crouch_aim_rifle_blend1" "crouch_aim_rifle_blend2" "crouch_aim_rifle_blend3" "crouch_aim_rifle_blend4" "crouch_aim_rifle_blend5" "crouch_aim_rifle_blend6" "crouch_aim_rifle_blend7" "crouch_aim_rifle_blend8" "crouch_aim_rifle_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_rifle "crouch_shoot_rifle_blend1" "crouch_shoot_rifle_blend2" "crouch_shoot_rifle_blend3" "crouch_shoot_rifle_blend4" "crouch_shoot_rifle_blend5" "crouch_shoot_rifle_blend6" "crouch_shoot_rifle_blend7" "crouch_shoot_rifle_blend8" "crouch_shoot_rifle_blend9" fps 20 blend XR -90 90 { event 5001 0 "30" } 
$sequence crouch_reload_rifle "crouch_reload_rifle" fps 30 
$sequence ref_aim_rifle "ref_aim_rifle_blend1" "ref_aim_rifle_blend2" "ref_aim_rifle_blend3" "ref_aim_rifle_blend4" "ref_aim_rifle_blend5" "ref_aim_rifle_blend6" "ref_aim_rifle_blend7" "ref_aim_rifle_blend8" "ref_aim_rifle_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_rifle "ref_shoot_rifle_blend1" "ref_shoot_rifle_blend2" "ref_shoot_rifle_blend3" "ref_shoot_rifle_blend4" "ref_shoot_rifle_blend5" "ref_shoot_rifle_blend6" "ref_shoot_rifle_blend7" "ref_shoot_rifle_blend8" "ref_shoot_rifle_blend9" fps 20 blend XR -90 90 { event 5001 0 "30" } 
$sequence ref_reload_rifle "ref_reload_rifle" fps 30 
$sequence crouch_aim_mp5 "crouch_aim_mp5_blend1" "crouch_aim_mp5_blend2" "crouch_aim_mp5_blend3" "crouch_aim_mp5_blend4" "crouch_aim_mp5_blend5" "crouch_aim_mp5_blend6" "crouch_aim_mp5_blend7" "crouch_aim_mp5_blend8" "crouch_aim_mp5_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_mp5 "crouch_shoot_mp5_blend1" "crouch_shoot_mp5_blend2" "crouch_shoot_mp5_blend3" "crouch_shoot_mp5_blend4" "crouch_shoot_mp5_blend5" "crouch_shoot_mp5_blend6" "crouch_shoot_mp5_blend7" "crouch_shoot_mp5_blend8" "crouch_shoot_mp5_blend9" fps 15 blend XR -90 90 { event 5001 0 "30" } 
$sequence crouch_reload_mp5 "crouch_reload_mp5" fps 30 
$sequence ref_aim_mp5 "ref_aim_mp5_blend1" "ref_aim_mp5_blend2" "ref_aim_mp5_blend3" "ref_aim_mp5_blend4" "ref_aim_mp5_blend5" "ref_aim_mp5_blend6" "ref_aim_mp5_blend7" "ref_aim_mp5_blend8" "ref_aim_mp5_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_mp5 "ref_shoot_mp5_blend1" "ref_shoot_mp5_blend2" "ref_shoot_mp5_blend3" "ref_shoot_mp5_blend4" "ref_shoot_mp5_blend5" "ref_shoot_mp5_blend6" "ref_shoot_mp5_blend7" "ref_shoot_mp5_blend8" "ref_shoot_mp5_blend9" fps 15 blend XR -90 90 { event 5001 0 "30" } 
$sequence ref_reload_mp5 "ref_reload_mp5" fps 30 
$sequence crouch_aim_shotgun "crouch_aim_shotgun_blend1" "crouch_aim_shotgun_blend2" "crouch_aim_shotgun_blend3" "crouch_aim_shotgun_blend4" "crouch_aim_shotgun_blend5" "crouch_aim_shotgun_blend6" "crouch_aim_shotgun_blend7" "crouch_aim_shotgun_blend8" "crouch_aim_shotgun_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_shotgun "crouch_shoot_shotgun_blend1" "crouch_shoot_shotgun_blend2" "crouch_shoot_shotgun_blend3" "crouch_shoot_shotgun_blend4" "crouch_shoot_shotgun_blend5" "crouch_shoot_shotgun_blend6" "crouch_shoot_shotgun_blend7" "crouch_shoot_shotgun_blend8" "crouch_shoot_shotgun_blend9" fps 30 blend XR -90 90 { event 5001 0 "50" } 
$sequence crouch_reload_shotgun "crouch_reload_shotgun" fps 30 
$sequence ref_aim_shotgun "ref_aim_shotgun_blend1" "ref_aim_shotgun_blend2" "ref_aim_shotgun_blend3" "ref_aim_shotgun_blend4" "ref_aim_shotgun_blend5" "ref_aim_shotgun_blend6" "ref_aim_shotgun_blend7" "ref_aim_shotgun_blend8" "ref_aim_shotgun_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_shotgun "ref_shoot_shotgun_blend1" "ref_shoot_shotgun_blend2" "ref_shoot_shotgun_blend3" "ref_shoot_shotgun_blend4" "ref_shoot_shotgun_blend5" "ref_shoot_shotgun_blend6" "ref_shoot_shotgun_blend7" "ref_shoot_shotgun_blend8" "ref_shoot_shotgun_blend9" fps 30 blend XR -90 90 { event 5001 0 "50" } 
$sequence ref_reload_shotgun "ref_reload_shotgun" fps 30 
$sequence crouch_aim_m249 "crouch_aim_m249_blend1" "crouch_aim_m249_blend2" "crouch_aim_m249_blend3" "crouch_aim_m249_blend4" "crouch_aim_m249_blend5" "crouch_aim_m249_blend6" "crouch_aim_m249_blend7" "crouch_aim_m249_blend8" "crouch_aim_m249_blend9" loop fps 10 blend XR -90 90 
$sequence crouch_shoot_m249 "crouch_shoot_m249_blend1" "crouch_shoot_m249_blend2" "crouch_shoot_m249_blend3" "crouch_shoot_m249_blend4" "crouch_shoot_m249_blend5" "crouch_shoot_m249_blend6" "crouch_shoot_m249_blend7" "crouch_shoot_m249_blend8" "crouch_shoot_m249_blend9" loop fps 30 blend XR -90 90 { event 5001 0 "50" } 
$sequence crouch_reload_m249 "crouch_reload_m249" fps 30 
$sequence ref_aim_m249 "ref_aim_m249_blend1" "ref_aim_m249_blend2" "ref_aim_m249_blend3" "ref_aim_m249_blend4" "ref_aim_m249_blend5" "ref_aim_m249_blend6" "ref_aim_m249_blend7" "ref_aim_m249_blend8" "ref_aim_m249_blend9" loop fps 10 blend XR -90 90 
$sequence ref_shoot_m249 "ref_shoot_m249_blend1" "ref_shoot_m249_blend2" "ref_shoot_m249_blend3" "ref_shoot_m249_blend4" "ref_shoot_m249_blend5" "ref_shoot_m249_blend6" "ref_shoot_m249_blend7" "ref_shoot_m249_blend8" "ref_shoot_m249_blend9" loop fps 30 blend XR -90 90 { event 5001 0 "50" } 
$sequence ref_reload_m249 "ref_reload_m249" fps 30 
$sequence I_am_a_stupid_placeholder "I_am_a_stupid_placeholder" loop fps 30 
$sequence so_am_I "so_am_I" fps 30 
$sequence ref_aim_grenade "ref_aim_grenade_blend1" "ref_aim_grenade_blend2" "ref_aim_grenade_blend3" "ref_aim_grenade_blend4" "ref_aim_grenade_blend5" "ref_aim_grenade_blend6" "ref_aim_grenade_blend7" "ref_aim_grenade_blend8" "ref_aim_grenade_blend9" loop fps 20 blend XR -90 90 
$sequence ref_shoot_grenade "ref_shoot_grenade_blend1" "ref_shoot_grenade_blend2" "ref_shoot_grenade_blend3" "ref_shoot_grenade_blend4" "ref_shoot_grenade_blend5" "ref_shoot_grenade_blend6" "ref_shoot_grenade_blend7" "ref_shoot_grenade_blend8" "ref_shoot_grenade_blend9" fps 45 blend XR -90 90 
$sequence crouch_aim_grenade "crouch_aim_grenade_blend1" "crouch_aim_grenade_blend2" "crouch_aim_grenade_blend3" "crouch_aim_grenade_blend4" "crouch_aim_grenade_blend5" "crouch_aim_grenade_blend6" "crouch_aim_grenade_blend7" "crouch_aim_grenade_blend8" "crouch_aim_grenade_blend9" loop fps 20 blend XR -90 90 
$sequence crouch_shoot_grenade "crouch_shoot_grenade_blend1" "crouch_shoot_grenade_blend2" "crouch_shoot_grenade_blend3" "crouch_shoot_grenade_blend4" "crouch_shoot_grenade_blend5" "crouch_shoot_grenade_blend6" "crouch_shoot_grenade_blend7" "crouch_shoot_grenade_blend8" "crouch_shoot_grenade_blend9" fps 45 blend XR -90 90 
$sequence crouch_aim_c4 "crouch_aim_c4_blend1" "crouch_aim_c4_blend2" "crouch_aim_c4_blend3" "crouch_aim_c4_blend4" "crouch_aim_c4_blend5" "crouch_aim_c4_blend6" "crouch_aim_c4_blend7" "crouch_aim_c4_blend8" "crouch_aim_c4_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_c4 "crouch_shoot_c4" fps 30 
$sequence ref_aim_c4 "ref_aim_c4_blend1" "ref_aim_c4_blend2" "ref_aim_c4_blend3" "ref_aim_c4_blend4" "ref_aim_c4_blend5" "ref_aim_c4_blend6" "ref_aim_c4_blend7" "ref_aim_c4_blend8" "ref_aim_c4_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_c4 "ref_shoot_c4" fps 30 
$sequence ref_reload_c4 "ref_reload_c4" fps 30 
$sequence crouch_aim_dualpistols "crouch_aim_dualpistols_blend1" "crouch_aim_dualpistols_blend2" "crouch_aim_dualpistols_blend3" "crouch_aim_dualpistols_blend4" "crouch_aim_dualpistols_blend5" "crouch_aim_dualpistols_blend6" "crouch_aim_dualpistols_blend7" "crouch_aim_dualpistols_blend8" "crouch_aim_dualpistols_blend9" loop fps 10 blend XR -90 90 
$sequence crouch_shoot_dualpistols "crouch_shoot_dualpistols_blend1" "crouch_shoot_dualpistols_blend2" "crouch_shoot_dualpistols_blend3" "crouch_shoot_dualpistols_blend4" "crouch_shoot_dualpistols_blend5" "crouch_shoot_dualpistols_blend6" "crouch_shoot_dualpistols_blend7" "crouch_shoot_dualpistols_blend8" "crouch_shoot_dualpistols_blend9" fps 30 blend XR -90 90 { event 5011 0 "10" } 
$sequence crouch_shoot2_dualpistols "crouch_shoot2_dualpistols_blend1" "crouch_shoot2_dualpistols_blend2" "crouch_shoot2_dualpistols_blend3" "crouch_shoot2_dualpistols_blend4" "crouch_shoot2_dualpistols_blend5" "crouch_shoot2_dualpistols_blend6" "crouch_shoot2_dualpistols_blend7" "crouch_shoot2_dualpistols_blend8" "crouch_shoot2_dualpistols_blend9" fps 30 blend XR -90 90 { event 5001 0 "10" } 
$sequence crouch_reload_dualpistols "crouch_reload_dualpistols" fps 30 
$sequence ref_aim_dualpistols "ref_aim_dualpistols_blend1" "ref_aim_dualpistols_blend2" "ref_aim_dualpistols_blend3" "ref_aim_dualpistols_blend4" "ref_aim_dualpistols_blend5" "ref_aim_dualpistols_blend6" "ref_aim_dualpistols_blend7" "ref_aim_dualpistols_blend8" "ref_aim_dualpistols_blend9" loop fps 10 blend XR -90 90 
$sequence ref_shoot_dualpistols "ref_shoot_dualpistols_blend1" "ref_shoot_dualpistols_blend2" "ref_shoot_dualpistols_blend3" "ref_shoot_dualpistols_blend4" "ref_shoot_dualpistols_blend5" "ref_shoot_dualpistols_blend6" "ref_shoot_dualpistols_blend7" "ref_shoot_dualpistols_blend8" "ref_shoot_dualpistols_blend9" fps 30 blend XR -90 90 { event 5011 0 "10" } 
$sequence ref_shoot2_dualpistols "ref_shoot2_dualpistols_blend1" "ref_shoot2_dualpistols_blend2" "ref_shoot2_dualpistols_blend3" "ref_shoot2_dualpistols_blend4" "ref_shoot2_dualpistols_blend5" "ref_shoot2_dualpistols_blend6" "ref_shoot2_dualpistols_blend7" "ref_shoot2_dualpistols_blend8" "ref_shoot2_dualpistols_blend9" fps 30 blend XR -90 90 { event 5001 0 "10" } 
$sequence ref_reload_dualpistols "ref_reload_dualpistols" fps 30 
$sequence crouch_aim_knife "crouch_aim_knife_blend1" "crouch_aim_knife_blend2" "crouch_aim_knife_blend3" "crouch_aim_knife_blend4" "crouch_aim_knife_blend5" "crouch_aim_knife_blend6" "crouch_aim_knife_blend7" "crouch_aim_knife_blend8" "crouch_aim_knife_blend9" loop fps 12 blend XR -90 90 
$sequence crouch_shoot_knife "crouch_shoot_knife_blend1" "crouch_shoot_knife_blend2" "crouch_shoot_knife_blend3" "crouch_shoot_knife_blend4" "crouch_shoot_knife_blend5" "crouch_shoot_knife_blend6" "crouch_shoot_knife_blend7" "crouch_shoot_knife_blend8" "crouch_shoot_knife_blend9" fps 30 blend XR -90 90 
$sequence ref_aim_knife "ref_aim_knife_blend1" "ref_aim_knife_blend2" "ref_aim_knife_blend3" "ref_aim_knife_blend4" "ref_aim_knife_blend5" "ref_aim_knife_blend6" "ref_aim_knife_blend7" "ref_aim_knife_blend8" "ref_aim_knife_blend9" loop fps 12 blend XR -90 90 
$sequence ref_shoot_knife "ref_shoot_knife_blend1" "ref_shoot_knife_blend2" "ref_shoot_knife_blend3" "ref_shoot_knife_blend4" "ref_shoot_knife_blend5" "ref_shoot_knife_blend6" "ref_shoot_knife_blend7" "ref_shoot_knife_blend8" "ref_shoot_knife_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_aim_ak47 "crouch_aim_ak47_blend1" "crouch_aim_ak47_blend2" "crouch_aim_ak47_blend3" "crouch_aim_ak47_blend4" "crouch_aim_ak47_blend5" "crouch_aim_ak47_blend6" "crouch_aim_ak47_blend7" "crouch_aim_ak47_blend8" "crouch_aim_ak47_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_ak47 "crouch_shoot_ak47_blend1" "crouch_shoot_ak47_blend2" "crouch_shoot_ak47_blend3" "crouch_shoot_ak47_blend4" "crouch_shoot_ak47_blend5" "crouch_shoot_ak47_blend6" "crouch_shoot_ak47_blend7" "crouch_shoot_ak47_blend8" "crouch_shoot_ak47_blend9" fps 30 blend XR -90 90 { event 5001 0 "40" } 
$sequence crouch_reload_ak47 "crouch_reload_ak47" fps 30 
$sequence ref_aim_ak47 "ref_aim_ak47_blend1" "ref_aim_ak47_blend2" "ref_aim_ak47_blend3" "ref_aim_ak47_blend4" "ref_aim_ak47_blend5" "ref_aim_ak47_blend6" "ref_aim_ak47_blend7" "ref_aim_ak47_blend8" "ref_aim_ak47_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_ak47 "ref_shoot_ak47_blend1" "ref_shoot_ak47_blend2" "ref_shoot_ak47_blend3" "ref_shoot_ak47_blend4" "ref_shoot_ak47_blend5" "ref_shoot_ak47_blend6" "ref_shoot_ak47_blend7" "ref_shoot_ak47_blend8" "ref_shoot_ak47_blend9" fps 30 blend XR -90 90 { event 5001 0 "40" } 
$sequence ref_reload_ak47 "ref_reload_ak47" fps 30 
$sequence crouch_aim_shieldgren "crouch_aim_shieldgren_blend1" "crouch_aim_shieldgren_blend2" "crouch_aim_shieldgren_blend3" "crouch_aim_shieldgren_blend4" "crouch_aim_shieldgren_blend5" "crouch_aim_shieldgren_blend6" "crouch_aim_shieldgren_blend7" "crouch_aim_shieldgren_blend8" "crouch_aim_shieldgren_blend9" loop fps 20 blend XR -90 90 
$sequence crouch_shoot_shieldgren "crouch_shoot_shieldgren_blend1" "crouch_shoot_shieldgren_blend2" "crouch_shoot_shieldgren_blend3" "crouch_shoot_shieldgren_blend4" "crouch_shoot_shieldgren_blend5" "crouch_shoot_shieldgren_blend6" "crouch_shoot_shieldgren_blend7" "crouch_shoot_shieldgren_blend8" "crouch_shoot_shieldgren_blend9" fps 45 blend XR -90 90 
$sequence ref_aim_shieldgren "ref_aim_shieldgren_blend1" "ref_aim_shieldgren_blend2" "ref_aim_shieldgren_blend3" "ref_aim_shieldgren_blend4" "ref_aim_shieldgren_blend5" "ref_aim_shieldgren_blend6" "ref_aim_shieldgren_blend7" "ref_aim_shieldgren_blend8" "ref_aim_shieldgren_blend9" loop fps 20 blend XR -90 90 
$sequence ref_shoot_shieldgren "ref_shoot_shieldgren_blend1" "ref_shoot_shieldgren_blend2" "ref_shoot_shieldgren_blend3" "ref_shoot_shieldgren_blend4" "ref_shoot_shieldgren_blend5" "ref_shoot_shieldgren_blend6" "ref_shoot_shieldgren_blend7" "ref_shoot_shieldgren_blend8" "ref_shoot_shieldgren_blend9" fps 45 blend XR -90 90 
$sequence crouch_aim_shieldknife "crouch_aim_shieldknife_blend1" "crouch_aim_shieldknife_blend2" "crouch_aim_shieldknife_blend3" "crouch_aim_shieldknife_blend4" "crouch_aim_shieldknife_blend5" "crouch_aim_shieldknife_blend6" "crouch_aim_shieldknife_blend7" "crouch_aim_shieldknife_blend8" "crouch_aim_shieldknife_blend9" loop fps 12 blend XR -90 90 
$sequence crouch_shoot_shieldknife "crouch_shoot_shieldknife_blend1" "crouch_shoot_shieldknife_blend2" "crouch_shoot_shieldknife_blend3" "crouch_shoot_shieldknife_blend4" "crouch_shoot_shieldknife_blend5" "crouch_shoot_shieldknife_blend6" "crouch_shoot_shieldknife_blend7" "crouch_shoot_shieldknife_blend8" "crouch_shoot_shieldknife_blend9" fps 30 blend XR -90 90 
$sequence ref_aim_shieldknife "ref_aim_shieldknife_blend1" "ref_aim_shieldknife_blend2" "ref_aim_shieldknife_blend3" "ref_aim_shieldknife_blend4" "ref_aim_shieldknife_blend5" "ref_aim_shieldknife_blend6" "ref_aim_shieldknife_blend7" "ref_aim_shieldknife_blend8" "ref_aim_shieldknife_blend9" loop fps 12 blend XR -90 90 
$sequence ref_shoot_shieldknife "ref_shoot_shieldknife_blend1" "ref_shoot_shieldknife_blend2" "ref_shoot_shieldknife_blend3" "ref_shoot_shieldknife_blend4" "ref_shoot_shieldknife_blend5" "ref_shoot_shieldknife_blend6" "ref_shoot_shieldknife_blend7" "ref_shoot_shieldknife_blend8" "ref_shoot_shieldknife_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_aim_shieldgun "crouch_aim_shieldgun_blend1" "crouch_aim_shieldgun_blend2" "crouch_aim_shieldgun_blend3" "crouch_aim_shieldgun_blend4" "crouch_aim_shieldgun_blend5" "crouch_aim_shieldgun_blend6" "crouch_aim_shieldgun_blend7" "crouch_aim_shieldgun_blend8" "crouch_aim_shieldgun_blend9" loop fps 30 blend XR -90 90 
$sequence crouch_shoot_shieldgun "crouch_shoot_shieldgun_blend1" "crouch_shoot_shieldgun_blend2" "crouch_shoot_shieldgun_blend3" "crouch_shoot_shieldgun_blend4" "crouch_shoot_shieldgun_blend5" "crouch_shoot_shieldgun_blend6" "crouch_shoot_shieldgun_blend7" "crouch_shoot_shieldgun_blend8" "crouch_shoot_shieldgun_blend9" fps 30 blend XR -90 90 { event 5001 0 "10" } 
$sequence crouch_reload_shieldgun "crouch_reload_shieldgun" fps 30 
$sequence ref_aim_shieldgun "ref_aim_shieldgun_blend1" "ref_aim_shieldgun_blend2" "ref_aim_shieldgun_blend3" "ref_aim_shieldgun_blend4" "ref_aim_shieldgun_blend5" "ref_aim_shieldgun_blend6" "ref_aim_shieldgun_blend7" "ref_aim_shieldgun_blend8" "ref_aim_shieldgun_blend9" loop fps 30 blend XR -90 90 
$sequence ref_shoot_shieldgun "ref_shoot_shieldgun_blend1" "ref_shoot_shieldgun_blend2" "ref_shoot_shieldgun_blend3" "ref_shoot_shieldgun_blend4" "ref_shoot_shieldgun_blend5" "ref_shoot_shieldgun_blend6" "ref_shoot_shieldgun_blend7" "ref_shoot_shieldgun_blend8" "ref_shoot_shieldgun_blend9" fps 30 blend XR -90 90 { event 5001 0 "10" } 
$sequence ref_reload_shieldgun "ref_reload_shieldgun" fps 30 
$sequence crouch_aim_shielded "crouch_aim_shielded_blend1" "crouch_aim_shielded_blend2" "crouch_aim_shielded_blend3" "crouch_aim_shielded_blend4" "crouch_aim_shielded_blend5" "crouch_aim_shielded_blend6" "crouch_aim_shielded_blend7" "crouch_aim_shielded_blend8" "crouch_aim_shielded_blend9" loop fps 20 blend XR -90 90 
$sequence ref_aim_shielded "ref_aim_shielded_blend1" "ref_aim_shielded_blend2" "ref_aim_shielded_blend3" "ref_aim_shielded_blend4" "ref_aim_shielded_blend5" "ref_aim_shielded_blend6" "ref_aim_shielded_blend7" "ref_aim_shielded_blend8" "ref_aim_shielded_blend9" loop fps 20 blend XR -90 90 
$sequence gut_flinch "gut_flinch_blend1" "gut_flinch_blend2" "gut_flinch_blend3" "gut_flinch_blend4" "gut_flinch_blend5" "gut_flinch_blend6" "gut_flinch_blend7" "gut_flinch_blend8" "gut_flinch_blend9" fps 30 blend XR -90 90 
$sequence head_flinch "head_flinch_blend1" "head_flinch_blend2" "head_flinch_blend3" "head_flinch_blend4" "head_flinch_blend5" "head_flinch_blend6" "head_flinch_blend7" "head_flinch_blend8" "head_flinch_blend9" fps 30 blend XR -90 90 
$sequence death1 "death1" fps 30 ACT_DIE_CHESTSHOT 1 { event 2001 1 } 
$sequence death2 "death2" fps 30 ACT_DIESIMPLE 2 { event 2001 1 } 
$sequence death3 "death3" fps 30 ACT_DIE_BACKSHOT 1 { event 2001 1 } 
$sequence head "head" fps 30 ACT_DIE_HEADSHOT 1 { event 2001 1 } 
$sequence gutshot "gutshot" fps 30 ACT_DIE_GUTSHOT 1 { event 2001 1 } 
$sequence left "left" fps 30 ACT_DIESIMPLE 1 { event 2001 1 } 
$sequence back "back" fps 30 ACT_DIEBACKWARD 1 { event 2001 1 } 
$sequence right "right" fps 30 ACT_DIESIMPLE 1 { event 2001 1 } 
$sequence forward "forward" fps 30 ACT_DIEFORWARD 1 { event 2001 1 } 
$sequence crouch_die "crouch_die" fps 30 { event 2001 1 } 
