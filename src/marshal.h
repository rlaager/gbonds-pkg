
#ifndef __gb_marshal_MARSHAL_H__
#define __gb_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:VOID (marshal.list:1) */
#define gb_marshal_VOID__VOID	g_cclosure_marshal_VOID__VOID

/* VOID:BOOLEAN (marshal.list:2) */
#define gb_marshal_VOID__BOOLEAN	g_cclosure_marshal_VOID__BOOLEAN

/* VOID:INT (marshal.list:3) */
#define gb_marshal_VOID__INT	g_cclosure_marshal_VOID__INT

/* VOID:INT,INT (marshal.list:4) */
extern void gb_marshal_VOID__INT_INT (GClosure     *closure,
                                      GValue       *return_value,
                                      guint         n_param_values,
                                      const GValue *param_values,
                                      gpointer      invocation_hint,
                                      gpointer      marshal_data);

/* VOID:INT,DOUBLE (marshal.list:5) */
extern void gb_marshal_VOID__INT_DOUBLE (GClosure     *closure,
                                         GValue       *return_value,
                                         guint         n_param_values,
                                         const GValue *param_values,
                                         gpointer      invocation_hint,
                                         gpointer      marshal_data);

/* VOID:DOUBLE (marshal.list:6) */
#define gb_marshal_VOID__DOUBLE	g_cclosure_marshal_VOID__DOUBLE

/* VOID:DOUBLE,DOUBLE (marshal.list:7) */
extern void gb_marshal_VOID__DOUBLE_DOUBLE (GClosure     *closure,
                                            GValue       *return_value,
                                            guint         n_param_values,
                                            const GValue *param_values,
                                            gpointer      invocation_hint,
                                            gpointer      marshal_data);

/* VOID:STRING (marshal.list:8) */
#define gb_marshal_VOID__STRING	g_cclosure_marshal_VOID__STRING

/* BOOLEAN:OBJECT (marshal.list:9) */
extern void gb_marshal_BOOLEAN__OBJECT (GClosure     *closure,
                                        GValue       *return_value,
                                        guint         n_param_values,
                                        const GValue *param_values,
                                        gpointer      invocation_hint,
                                        gpointer      marshal_data);

G_END_DECLS

#endif /* __gb_marshal_MARSHAL_H__ */

