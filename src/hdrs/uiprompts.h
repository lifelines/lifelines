#ifndef INCLUDED_UIPROMPTS_H
#define INCLUDED_UIPROMPTS_H

/* ask to ensure user got to see the indi */
typedef INT CONFIRMQ;
#define DOCONFIRM 1
#define NOCONFIRM 0

/* whether to prompt for new child if none existing */
typedef INT PROMPTQ;
#define ALWAYS_PROMPT 0
#define PROMPT_IF_CHILDREN 1

/* ask if only one match */
typedef INT ASK1Q;
#define DOASK1 1
#define NOASK1 0


#endif /* INCLUDED_UIPROMPTS_H */
