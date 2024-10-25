Student name: Asher Arya

Comments to TA about your implementation:
- I reset points on game restart, and reset timers for spawns

## Custom Features
SpeedUp:
- I added the speeding up in main
Advanced Mode:
- I added whirlpools that attract everything and kill everything that touches them (playing a death sound when other entities die). They despawn after WHIRLPOOL_DEATH_TIMER. They have a random force that drawns in everything nearby. They also have a random radius of effect and size.
- I also took the creative liberty of reducing the massive salmon's size, since it made the game unplayable in advanced mode.
- I also added a pufferfish that traverses from the bottom of the screen to one of the sides via random arcs (via acceleration). it is fast and hard to catch but worth 5 points instead of 1
- I modified the way deathtimers work to be able to use them for whirlpools and entities that die on collision with whirlpools
## Note:
Make sure to delete your .vs and out folders before submitting your assignment.