// Case for the QRemoteBox
// Author: Alexander Rössler (mail.aroessler@gmail.com) 
// License: Creative Commons

/**
 * @id qremotebox_case
 * @name Case for the QRemoteBox
 * @category Printed
 */

sideSpacing = 0.5;

boardWidth = 62;
boardLength = 62;
boardHeight = 1.5;

thickness = 2;

baseWidth = boardWidth+sideSpacing*2;
baseLength = boardLength+sideSpacing*2;
baseHeight = thickness;
hole1X = 100*0.0254;
hole1Y = 2350*0.0254;
hole1D = 2.2;
hole1H = 6;
hole2X = 2350*0.0254;
hole2Y = 100*0.0254;
hole2D = 2.2;
hole2H = hole1H;
corner1W = 2.5;
corner1L = 2.5;
corner1H = 3.5;
corner2W = 4.5;
corner2L = 4.5;
corner2H = corner1H;
corner3W = 2.5;
corner3L = 2.5;
corner3H = corner1H;
corner4W = 4.5;
corner4L = 4.5;
corner4H = corner1H;

/* box parameters */
boxWidth = baseWidth + thickness*2 + 21;
boxLength = baseLength + thickness*2;
boxHeight = 20;
intersectionThickness = 1;
intersectionHeight = 2;
intersectionSpacing = 0.25;
/* design parameters */
designThickness = 2;

/* screw corners and holes */
screwHull = 2;
screwCornerBottomW = 8.5;
screwCornerBottomL = 7.5;
screwCornerBottomH = corner1H;
screwCornerBottomTopW = 8;
screwCornerBottomTopL = 7.5;
screwCornerBottomTopH = intersectionHeight+boxHeight/2-screwCornerBottomH-thickness;
screwCornerTopW = screwCornerBottomTopW;
screwCornerTopL = screwCornerBottomTopL;
screwCornerTopH = boxHeight/2-thickness-intersectionHeight-intersectionSpacing;

boxConnectorW = 5;
boxConnectorL = 2;
boxConnectorH = 11;
boxConnectorTopW = boxConnectorW;
boxConnectorTopL = 3;
boxConnectorTopH = 4;
boxConnectorHoleW = 6;
boxConnectorHoleL = 10;
boxConnectorHoleH = 3;
boxConnector1X = -boxConnectorW+sideSpacing;
boxConnector1Y = 0;
boxConnector1Z = thickness;
boxConnector2X = boardWidth+sideSpacing;
boxConnector2Y = 0;
boxConnector2Z = thickness;
boxConnector3X = boxConnector1X;
boxConnector3Y = boxLength-thickness*2;
boxConnector3Z = thickness;
boxConnector4X = boxConnector2X;
boxConnector4Y = boxLength-thickness*2;
boxConnector4Z = thickness;

/* led shafts and holes */
ledShaftThickness = 1;
ledShaftWall1L = 16;
ledBoardSpacing = 0.5;
ledShaftWallH = boxHeight - thickness*2 - boardHeight - corner1H - intersectionSpacing - ledBoardSpacing;
ledShaftWall2W = 5;
ledShaftSpacing = (ledShaftWall1L-ledShaftThickness)/4;
ledShaftX = 22.5;
ledShaftY = 45;
ledHoleX = 25;
ledHoleY = 47.5;
ledHoleRadius = 1;

/* buttons */
buttonPhyH = 0.55;
buttonW = 8;
buttonL = 6;
buttonBridge = buttonL-2;
buttonBridgeDepth = 0.5;
buttonTopSpacing = 0.5;
buttonBoardSpacing = 0.25;
buttonRoundness = 1;
buttonRadius = 1.5;
buttonRadius2 = 1;
buttonH = boxHeight - thickness*2 - boardHeight - corner1H - intersectionSpacing - buttonPhyH - buttonBoardSpacing;
button1X = 52;
button1Y = 31;
button2X = 52;
button2Y = 23;
button3X = 52;
button3Y = 15.5;

/* pusher */
pusherRadius = 2;
pusherSpacing = 0.75;
pusherH = boxHeight - thickness*2 - boardHeight - corner1H - intersectionSpacing - pusherSpacing;
pusher1X = 25;
pusher1Y = 35;
pusher2X = 43;
pusher2Y = 15;

/* logo */
useLogo=false;
logoW = 20;
logoL = 20;
logoDepth = 0.8;
logoX = -1;
logoY = 41;

/* m3 screw */
m3=4;
m3nut1=5.9;
m3nut2=6.3;
m3nutHeight=3;
m3head=6.5;
m3headHeight=3;
m3length=boxHeight-thickness-m3headHeight*1.5;


// project settings
showBottom = false;
showTop = true;
showBoard = false;

// print settings
mirrorTop = true;

// render settings
$fn=200;
//$fn=10;
$fs=0.8; // def 1, 0.2 is high res
$fa=4;   //def 12, 3 is very nice
cgalfix=0.01;

if (showBoard) {
    translate([sideSpacing,sideSpacing,corner1H+baseHeight]) board();
}
 
rotate([0,(((mirrorTop && showTop && (!showBottom) && (!showBoard))?180:0)),0])
//color([1.0,1.0,1.0,0.1])
color("white")
difference() {
    union() {
        /* box */
        if (showTop)
        difference() {
            union() {
                /* LED shaft */
                translate([ledShaftX,ledShaftY,thickness+boardHeight+corner1H+intersectionSpacing+ledBoardSpacing+cgalfix]) cube([ledShaftThickness, ledShaftWall1L, ledShaftWallH]);
                translate([ledShaftX+ledShaftWall2W-ledShaftThickness,ledShaftY,thickness+boardHeight+corner1H+intersectionSpacing+ledBoardSpacing+cgalfix]) cube([ledShaftThickness, ledShaftWall1L, ledShaftWallH]);
                translate([ledShaftX,ledShaftY,thickness+boardHeight+corner1H+intersectionSpacing+ledBoardSpacing+cgalfix]) cube([ledShaftWall2W, ledShaftThickness, ledShaftWallH]);
                for (i = [0:4]) {
                    translate([ledShaftX,ledShaftY+ledShaftSpacing*i,thickness+boardHeight+corner1H+intersectionSpacing+ledBoardSpacing+cgalfix]) cube([ledShaftWall2W, ledShaftThickness, ledShaftWallH]);
                }

                /* buttons */
                buttonAdditive();

                /* pusher */
                translate([pusher1X,pusher1Y,thickness+boardHeight+corner1H+intersectionSpacing+pusherSpacing+cgalfix]) cylinder(r=pusherRadius, h=pusherH);
                translate([pusher2X,pusher2Y,thickness+boardHeight+corner1H+intersectionSpacing+pusherSpacing+cgalfix]) cylinder(r=pusherRadius, h=pusherH);
        
                /* screw corners */ /*
                translate([baseWidth,0,boxHeight-thickness-screwCornerTopH]) 
                    difference() {
                        cube([screwCornerTopW,screwCornerTopL,screwCornerTopH]);
                        translate([-cgalfix,-cgalfix,screwCornerTopH-m3nutHeight+cgalfix]) cube([m3nut2,m3nut1,m3nutHeight]);
                    }
                translate([-screwCornerBottomTopW,baseLength-screwCornerBottomTopL,boxHeight-thickness-screwCornerTopH]) 
                    difference() {
                        cube([screwCornerTopW,screwCornerTopL,screwCornerTopH]);
                        translate([screwCornerTopW-m3nut2+cgalfix,screwCornerTopL-m3nut1+cgalfix,screwCornerTopH-m3nutHeight+cgalfix]) cube([m3nut2,m3nut1,m3nutHeight]);
                    }
                */

                /* box */
                translate([-(boxWidth-baseWidth)/2,-(boxLength-baseLength)/2,0]) 
                    box(false);
            }
            union() {   // all things that should be removed from the top

                /* box connector */
                translate([boxConnector1X,boxConnector1Y,boxConnector1Z]) 
                    boxConnector(true);
                translate([boxConnector2X,boxConnector2Y,boxConnector2Z]) 
                    boxConnector(true);
                translate([boxConnector3X,boxConnector3Y,boxConnector3Z]) 
                    mirror([0,1,0]) boxConnector(true);
                translate([boxConnector4X,boxConnector4Y,boxConnector4Z]) 
                    mirror([0,1,0]) boxConnector(true);

                /* led holes */
                for (i = [0:3]) {
                    translate([ledHoleX,ledHoleY+ledShaftSpacing*i,boxHeight-thickness-1]) 
                        cylinder(r=ledHoleRadius, h=thickness+2);
                }

                /* button holes */
                buttonSubtraktive();

                /* logo */
                if (useLogo)
                translate([logoX,logoY,boxHeight-logoDepth])
                    resize([logoW,logoL,logoDepth+1]) linear_extrude(height = 2) import("qrc_logo2.dxf");
            
                /* ir detect correction */
                //translate([56.5+sideSpacing,57.5,boardHeight+corner1H+baseHeight+5]) 
                //    rotate([-90,0,0]) cylinder(r=3, h=10);

                /* ir rx correction */
                //translate([49+sideSpacing,57.5,boardHeight+corner1H+baseHeight+3.5]) 
                //    rotate([-90,0,0]) cylinder(r=3, h=10);
            }
        }
        if (showBottom)
        difference() {  
            union() {
                /* screw corner bottom */ /*
                translate([baseWidth-sideSpacing,0,thickness]) 
                    cube([screwCornerBottomW,screwCornerBottomL,screwCornerBottomH]);
                translate([baseWidth,0,thickness+screwCornerBottomH]) 
                    cube([screwCornerBottomTopW,screwCornerBottomTopL,screwCornerBottomTopH]); 
                translate([-screwCornerBottomW+sideSpacing,baseLength-screwCornerBottomL,thickness]) 
                    cube([screwCornerBottomW,screwCornerBottomL,screwCornerBottomH]);
                translate([-screwCornerBottomTopW,baseLength-screwCornerBottomTopL,thickness+screwCornerBottomH]) 
                    cube([screwCornerBottomTopW,screwCornerBottomTopL,screwCornerBottomTopH]);
                */

                /* box connector */
                translate([boxConnector1X,boxConnector1Y-cgalfix,boxConnector1Z-cgalfix]) 
                    boxConnector();
                translate([boxConnector2X,boxConnector2Y-cgalfix,boxConnector2Z-cgalfix]) 
                    boxConnector();
                translate([boxConnector3X,boxConnector3Y+cgalfix,boxConnector3Z-cgalfix]) 
                    mirror([0,1,0]) boxConnector();
                translate([boxConnector4X,boxConnector4Y+cgalfix,boxConnector4Z-cgalfix]) 
                    mirror([0,1,0]) boxConnector();
                
                /* stand */
                stand();
    
                /* box */
                translate([-(boxWidth-baseWidth)/2,-(boxLength-baseLength)/2,0]) 
                    box(true);
            }
            union() {   // all things that should be removed from the bottom
                
                /* irled1 correction */
                translate([55+sideSpacing,thickness,boardHeight+corner1H+baseHeight+4]) 
                    rotate([90,0,0]) cylinder(r=3, h=10);
                /* irled2 correction */
                translate([37.5+sideSpacing,57.5,boardHeight+corner1H+baseHeight+4]) 
                    rotate([-90,0,0]) cylinder(r=3, h=10);
            }
        }
    }
    if (showTop || (showBottom))
    union() {
        /* the screw holes */
/*
        translate([baseWidth+screwHull+sideSpacing*2,m3nut1/2,-cgalfix]) m3screw();
        translate([-screwHull-sideSpacing*2,baseLength-m3nut1/2,-cgalfix]) m3screw();
*/
        /* box holes */
        /* irled1 */
        translate([55+sideSpacing,thickness,boardHeight+corner1H+baseHeight+2.5]) 
            rotate([90,0,0]) cylinder(r=3, h=10);
        
        /* usb Connector */
        translate([39.5+sideSpacing,-11+cgalfix,boardHeight+corner1H+baseHeight-4.5+1.5]) 
            cube([11,10,10]);
        translate([40.5+sideSpacing,-5,boardHeight+corner1H+baseHeight-4.5+4.5]) 
            cube([9,10,3]);
        /* irled2 */
        translate([37.5+sideSpacing,57.5,boardHeight+corner1H+baseHeight+2.5]) 
            rotate([-90,0,0]) cylinder(r=3, h=10);
        /* ir rx */
        translate([49+sideSpacing,57.5,boardHeight+corner1H+baseHeight+6]) 
            rotate([-90,0,0]) cylinder(r=3, h=10);
        /* ir detect */
        translate([56.5+sideSpacing,57.5,boardHeight+corner1H+baseHeight+6.5]) 
            rotate([-90,0,0]) cylinder(r=3, h=10);
    }
}

module buttonAdditive()
{
    translate([button1X,button1Y,thickness+boardHeight+corner1H+intersectionSpacing+buttonPhyH+buttonBoardSpacing+cgalfix]) cylinder(r2=buttonRadius, r=buttonRadius2, h=buttonH);
    translate([button2X,button2Y,thickness+boardHeight+corner1H+intersectionSpacing+buttonPhyH+buttonBoardSpacing+cgalfix]) cylinder(r2=buttonRadius, r=buttonRadius2, h=buttonH);
    translate([button3X,button3Y,thickness+boardHeight+corner1H+intersectionSpacing+buttonPhyH+buttonBoardSpacing+cgalfix]) cylinder(r2=buttonRadius, r=buttonRadius2, h=buttonH);
}

module buttonSubtraktive()
{
    module subtraktivePart()
    {
        difference() {
            translate([-(buttonW/2+buttonTopSpacing),-(buttonL/2+buttonTopSpacing),0]) 
                roundedCube([buttonW+buttonTopSpacing*2, buttonL+buttonTopSpacing*2, thickness+1], buttonRoundness);
            translate([-(buttonW/2),-(buttonL/2),0]) 
                roundedCube([buttonW, buttonL, thickness+1], buttonRoundness);
            translate([buttonW/2-buttonTopSpacing+cgalfix,-buttonBridge/2,-buttonBridgeDepth])
                cube([buttonTopSpacing*2, buttonBridge, thickness]);
        }
    }
    translate([button1X,button1Y,boxHeight-thickness-cgalfix]) subtraktivePart();
    translate([button2X,button2Y,boxHeight-thickness-cgalfix]) subtraktivePart();
    translate([button3X,button3Y,boxHeight-thickness-cgalfix]) subtraktivePart();    
}
    
module box(bottom=true)
{
    module baseBox(lengthDifference, heightDifference)
    {
        union() {
                translate([boxHeight/2,-lengthDifference,-heightDifference])  cube([boxWidth-boxHeight,boxLength+lengthDifference*2,boxHeight+heightDifference*2]);
                translate([boxHeight/2,-lengthDifference,boxHeight/2]) rotate([-90,0,0]) cylinder(r=boxHeight/2+heightDifference, h=boxLength+lengthDifference*2);
                translate([boxWidth-boxHeight/2,-lengthDifference,boxHeight/2]) rotate([-90,0,0]) cylinder(r=boxHeight/2+heightDifference, h=boxLength+lengthDifference*2);
            }
    }
    
    module boxSection(start, stop)
    {
        translate([-5,-5,start]) cube([boxWidth+10,boxLength+10,stop-start]);
    }

    union()
    {
        difference()
        {
            /* outer box */
            baseBox(designThickness,0);
            /* inner box */
            baseBox(-thickness,-thickness);
            /* design */
            translate([0,-boxLength+designThickness,0]) baseBox(-thickness, -thickness);
            translate([0,boxLength-designThickness,0]) baseBox(-thickness, -thickness);
            
            if (!bottom) {
                difference() {
                    baseBox(-(thickness-intersectionThickness),-(thickness-intersectionThickness));
                    boxSection(-cgalfix, boxHeight/2-cgalfix);
                    boxSection(boxHeight/2+intersectionHeight+intersectionSpacing+cgalfix, boxHeight+cgalfix);
                }
            }
            /* split the box in 2 halfes */
            if (bottom) {
                boxSection(boxHeight/2,boxHeight+cgalfix);
            }
            else {
                boxSection(-cgalfix,boxHeight/2);
            }
        }
        if (bottom) {
            difference() {
                baseBox(-(thickness-intersectionThickness+intersectionSpacing),-(thickness-intersectionThickness+intersectionSpacing));
                /* inner box */
                baseBox(-thickness,-thickness);
                /* slicing */
                boxSection(-cgalfix,boxHeight/2);
                boxSection(boxHeight/2+intersectionHeight, boxHeight);
            }
        }
    }
}

module stand()
{
    translate([hole1X+sideSpacing,hole1Y+sideSpacing,baseHeight-cgalfix]) cylinder(r=hole1D/2, h=hole1H);
    translate([hole2X+sideSpacing,hole2Y+sideSpacing,baseHeight-cgalfix]) cylinder(r=hole2D/2, h=hole2H);
    translate([sideSpacing,0,baseHeight-cgalfix]) cube([corner1W,corner1L+sideSpacing,corner1H]);
    translate([sideSpacing,baseLength-corner2L-sideSpacing,baseHeight-cgalfix]) cube([corner2W,corner2L+sideSpacing,corner2H]);
    translate([baseWidth-corner3W-sideSpacing,baseLength-corner3L-sideSpacing,baseHeight-cgalfix]) cube([corner3W,corner3L+sideSpacing,corner3H]);
    translate([baseWidth-corner4W-sideSpacing,0,baseHeight-cgalfix]) cube([corner4W,corner4L+sideSpacing,corner4H]);
}

module boxConnector(hole=false)
{
    if (!hole)
    {
        union() {
            cube([boxConnectorW, boxConnectorL, boxConnectorH+boxConnectorTopH/2]);
            translate([-(boxConnectorTopW-boxConnectorW)/2,boxConnectorL,boxConnectorH+boxConnectorTopH/2-cgalfix]) 
                rotate([0,0,270]) prism(boxConnectorTopW,boxConnectorTopL,boxConnectorTopH/2);
            translate([boxConnectorW+(boxConnectorTopW-boxConnectorW)/2,cgalfix,boxConnectorH+boxConnectorTopH/2]) 
                rotate([180,0,270]) prism(boxConnectorTopW,boxConnectorTopL-boxConnectorL,boxConnectorTopH/2);
        }
    }
    else
    {
        translate([-(boxConnectorHoleW-boxConnectorW)/2,-boxConnectorHoleL/2,boxConnectorH+(boxConnectorTopH-boxConnectorHoleH)/2])
            cube([boxConnectorHoleW,boxConnectorHoleL,boxConnectorHoleH]);
    }
}

module m3screw() {
    union() {
            translate([0,0,m3headHeight-0.1]) cylinder(r=m3/2,h=m3length+0.1);
            translate([0,0,m3headHeight-0.1]) cylinder(r=m3head/2, r2=m3/2, h=m3headHeight/2+0.1);  // for better printing results
            translate([0,0,-0.1]) cylinder(r=m3head/2,h=m3headHeight+0.1);
    }
}

module board()
{
    antennaWidth1 = 13.7;
    antennaLength1 = 27.94;
    antennaHeight1 = 1.5;
    antennaPosX1 = -385*0.0254;
    antennaPosY1 = 275*0.0254;

    antennaWidth2 = 13.7;
    antennaLength2 = 27.94;
    antennaHeight2 = 1.5;
    antennaPosX2 = 2285*0.0254;
    antennaPosY2 = 1125*0.0254;

    antennaWidth3 = 9.14;
    antennaLength3 = 12.7;
    antennaHeight3 = 1.57;
    antennaPosX3 = -225*0.0254;
    antennaPosY3 = 1525*0.0254;

    switchWidth = 4.5;
    switchLength = 4.5;
    switchHeight = 0.55;
    switchPosX1 = 335*0.0254;
    switchPosY1 = 1110*0.0254;
    switchPosX2 = 1960*0.0254;
    switchPosY2 = 510*0.0254;
    switchPosX3 = 1960*0.0254;
    switchPosY3 = 810*0.0254;
    switchPosX4 = 1960*0.0254;
    switchPosY4 = 1110*0.0254;

    ledWidth = 2;
    ledLength = 1.2;
    ledHeight = 0.5;
    ledPosX1 = 935*0.0254;
    ledPosY1 = 2275*0.0254;
    ledPosX2 = 935*0.0254;
    ledPosY2 = 2125*0.0254;
    ledPosX3 = 935*0.0254;
    ledPosY3 = 1975*0.0254;
    ledPosX4 = 935*0.0254;
    ledPosY4 = 1825*0.0254;
    ledPosX5 = 1085*0.0254;
    ledPosY5 = 2350*0.0254;
    ledPosX6 = 1085*0.0254;
    ledPosY6 = 2200*0.0254;
    ledPosX7 = 1085*0.0254;
    ledPosY7 = 2050*0.0254;
    ledPosX8 = 1085*0.0254;
    ledPosY8 = 1900*0.0254;

    usbConnectorWidth = 300*0.0254;
    usbConnectorLength = 250*0.0254;
    usbConnectorHeight = 3;
    usbConnectorPosX = 1625*0.0254;
    usbConnectorPosY = -5*0.0254;

    irLedLength = 8.6;
    irLedRadius = 2.5;
    irLedPosX1 = 55;
    irLedPosY1 = 5;
    irLedPosX2 = 37.5;
    irLedPosY2 = 57.5;

    irRxWidth1 = 6;
    irRxLength1 = 4;
    irRxHeight1 = 8.5;
    irRxPosX1 = 46;
    irRxPosY1 = 57.3;

    irRxWidth2 = 4.5;
    irRxLength2 = 2.5;
    irRxHeight2 = 8.5;
    irRxPosX2 = 54;
    irRxPosY2 = 58.5;

    capacitorRadius = 4;
    capacitorHeight = 6;
    capacitorPosX1 = 350*0.0254;
    capacitorPosY1 = 2250*0.0254;
    capacitorPosX2 = 350*0.0254;
    capacitorPosY2 = 1875*0.0254;
    capacitorPosX3 = 350*0.0254;
    capacitorPosY3 = 1500*0.0254;
    capacitorPosX4 = 1175*0.0254;
    capacitorPosY4 = 475*0.0254;

    boxConnectorWidth1 = 120*0.0254;
    boxConnectorLength1 = 1225*0.0254;
    boxConnectorHeight1 = 5.5;
    connectorPosX1 = 610*0.0254;
    connectorPosY1 = 1010*0.0254;

    boxConnectorWidth2 = 250*0.0254;
    boxConnectorLength2 = 150*0.0254;
    boxConnectorHeight2 = 5;
    connectorPosX2 = 1625*0.0254;
    connectorPosY2 = 955*0.0254;

    ismWidth = 38;
    ismLength = 7;
    ismHeight = 11;
    ismPosX = 1.5;
    ismPosY = 3;
    

    color("green") cube([boardWidth,boardLength,boardHeight]);
    color([0.2,0.2,0.2,1.0]) translate([antennaPosX1,antennaPosY1,boardHeight]) cube([antennaWidth1, antennaLength1, antennaHeight1]);
    color([0.2,0.2,0.2,1.0]) translate([antennaPosX2,antennaPosY2,boardHeight]) cube([antennaWidth2, antennaLength2, antennaHeight2]);
    color([0.2,0.2,0.2,1.0]) translate([antennaPosX3,antennaPosY3,-boardHeight]) cube([antennaWidth3, antennaLength3, antennaHeight3]);
    color("yellow") translate([switchPosX1,switchPosY1,boardHeight]) cube([switchWidth, switchLength, switchHeight]);
    color("yellow") translate([switchPosX2,switchPosY2,boardHeight]) cube([switchWidth, switchLength, switchHeight]);
    color("yellow") translate([switchPosX3,switchPosY3,boardHeight]) cube([switchWidth, switchLength, switchHeight]);
    color("yellow") translate([switchPosX4,switchPosY4,boardHeight]) cube([switchWidth, switchLength, switchHeight]);
    color("yellow") translate([ledPosX1,ledPosY1,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("lightgreen") translate([ledPosX2,ledPosY2,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("yellow") translate([ledPosX3,ledPosY3,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("red") translate([ledPosX4,ledPosY4,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("yellow") translate([ledPosX5,ledPosY5,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("lightgreen") translate([ledPosX6,ledPosY6,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("yellow") translate([ledPosX7,ledPosY7,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("red") translate([ledPosX8,ledPosY8,boardHeight]) cube([ledWidth, ledLength, ledHeight]);
    color("grey") translate([usbConnectorPosX,usbConnectorPosY,boardHeight]) cube([usbConnectorWidth, usbConnectorLength, usbConnectorHeight]);
    color("blue") translate([irLedPosX1,irLedPosY1,boardHeight+irLedRadius]) rotate([90,0,0]) cylinder(r=irLedRadius, h=irLedLength);
    color("blue") translate([irLedPosX2,irLedPosY2,boardHeight+irLedRadius]) rotate([-90,0,0]) cylinder(r=irLedRadius, h=irLedLength);
    color([0.2,0.2,0.2,1.0]) translate([irRxPosX1,irRxPosY1,boardHeight]) cube([irRxWidth1, irRxLength1, irRxHeight1]);
    color([0.2,0.2,0.2,1.0]) translate([irRxPosX2,irRxPosY2,boardHeight]) cube([irRxWidth2, irRxLength2, irRxHeight2]);
    color("grey") translate([capacitorPosX1,capacitorPosY1,boardHeight]) cylinder(r=capacitorRadius, h=capacitorHeight);
    color("grey") translate([capacitorPosX2,capacitorPosY2,boardHeight]) cylinder(r=capacitorRadius, h=capacitorHeight);
    color("grey") translate([capacitorPosX3,capacitorPosY3,boardHeight]) cylinder(r=capacitorRadius, h=capacitorHeight);
    color("grey") translate([capacitorPosX4,capacitorPosY4,boardHeight]) cylinder(r=capacitorRadius, h=capacitorHeight);
    color("grey") translate([connectorPosX1,connectorPosY1,boardHeight]) cube([boxConnectorWidth1, boxConnectorLength1, boxConnectorHeight1]);
    color("grey") translate([connectorPosX2,connectorPosY2,boardHeight]) cube([boxConnectorWidth2, boxConnectorLength2, boxConnectorHeight2]);
    color("grey") translate([ismPosX,ismPosY,boardHeight]) cube([ismWidth, ismLength, ismHeight]);
}

module roundedCube(size, radius)
{
x = size[0];
y = size[1];
z = size[2];

translate([x/2,y/2,0])
linear_extrude(height=z)
hull()
        {
        // place 4 circles in the corners, with the given radius
        translate([(-x/2)+(radius), (-y/2)+(radius), 0])
        circle(r=radius);

        translate([(x/2)-(radius), (-y/2)+(radius), 0])
        circle(r=radius);

        translate([(-x/2)+(radius), (y/2)-(radius), 0])
        circle(r=radius);

        translate([(x/2)-(radius), (y/2)-(radius), 0])
        circle(r=radius);
        }
}

module prism(l, w, h) {
        translate([0, l, 0]) rotate( a= [90, 0, 0]) 
        linear_extrude(height = l) polygon(points = [
                [0, 0],
                [w, 0],
                [0, h]
        ], paths=[[0,1,2,0]]);
}