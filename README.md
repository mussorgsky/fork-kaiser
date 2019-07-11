# fork-kaiser *(a.k.a. Forky)*

Program for controlling a line-following, stock-keeping forklift robot, made of LEGO using a Husarion Core2 microcontroller.

## The design of the robot was as follows:
- Two separately driven front wheels (tank steering)
- A castor-style wheel in the back
- Two light reflection sensors for detecting the line (initially three, but one got fried 😢)
- A lift that could put tiny pallets on two different levels on a storage rack
- A color sensor for detecting which kind of pallet was currently picked up

## Pictures:
>![Real life Forky](https://i.imgur.com/3rI8kng.jpg)
>
> Forky in real life

>![Visualization of the track](https://i.imgur.com/HYd3no4.jpg)
>
> Digital design of the track

## Things this program does:
- Drive along a line and track passed "checkpoints" to know where it is
- Attempt to pick up and recognize a pallet
- If it recognizes the pallet and there is enough space on the rack, it will carry it there
- Otherwise it will put it back down and await user confirmation before continuing

## Things this program doesn't do:
- Look nice, it's written in the most straight forward, fragile way ever
- Try to return to the track after getting lost, if it gets lost it's gone for good. During the presentation for grading, the robot managed to run for almost *two hours*, filling the storage "facility" many times and only got lost *once* though

### Epitaph
Forky was decommissioned on 2019-06-05 at 11:57 following successful evaluation

Gone but not forgotten
