#ifndef LET_EVAL_H
#define LET_EVAL_H

struct letdata *global_eval(struct block *root,
			    struct env *local_env,
			    struct env *global_env);

#endif	/* LET_EVAL_H */
