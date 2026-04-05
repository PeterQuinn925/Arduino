//device dimensions
//63.64 * 28.0 * 9mm
//without USB port or pins
//battery 26x20x4
//display 23 x 13
//total thickness needed = 18mm
//
//top half is just a thin box with a hole for the display

overallX = 75;
overallY = 35;
bothalfZ = 2;
winX = 23;
winY = 13;

difference(){
    cube([overallX,overallY,bothalfZ],center=true);
translate([6,0,-2])
    cube([winX,winY,10],center=true);}