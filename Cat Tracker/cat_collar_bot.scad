//device dimensions
//63.64 * 28.0 * 9mm
//without USB port or pins
//battery 26x20x4
//display 23 x 13
//total thickness needed = 18mm
//
//bottom half is a open box with wings for attaching to a strap


//TODO. need a hole for the USB port
overallX = 75;
overallY = 35;
tophalfZ = 15;

difference() {
cube([overallX,overallY,tophalfZ],center=true);
translate([0,0,2])
    cube([66,30,15],center=true);//hole for device & batt
translate([-overallX/2,0,2])
    cube([10,9,3],center=true);//hole for USB
}
//make a hole for the battery or more accuratly, fill it.
translate([14,0,-4])
    cube([66-26,30,4],center=true);
//right strap connector
difference() {
translate([(75+10)/2,0,6])
    cube([10,35,3],center=true);
translate([(75+10)/2,0,10/2])
    cube([4,30,7],center=true);
}
//left strap connector
difference() {
translate([-(75+10)/2,0,6])
    cube([10,35,3],center=true);
translate([-(75+10)/2,0,10/2])
    cube([4,30,7],center=true);
}