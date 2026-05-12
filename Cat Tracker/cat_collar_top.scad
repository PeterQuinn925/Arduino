//device dimensions
//63.64 * 28.0 * 9mm
//without USB port or pins
//battery 26x20x4
//display 23 x 13
//total thickness needed = 18mm
//
//top half is just a thin box with a hole for the display
// M2 screws

overallX = 75;
overallY = 35;
bothalfZ = 2;
hole_r = 1;
hole_offset = 2;

difference(){
    cube([overallX,overallY,bothalfZ],center=true);
translate([overallX/2-hole_offset,overallY/2-hole_offset,-2])
    cylinder(10,hole_r,hole_r);
translate([-overallX/2+hole_offset,overallY/2-hole_offset,-2])
    cylinder(10,hole_r,hole_r);
translate([-overallX/2+hole_offset,-overallY/2+hole_offset,-2])
    cylinder(10,hole_r,hole_r);
translate([overallX/2-hole_offset,-overallY/2+hole_offset,-2])
    cylinder(10,hole_r,hole_r);
    }
