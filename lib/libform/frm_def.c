/*-----------------------------------------------------------------------------+
|           The ncurses form library is  Copyright (C) 1995-1997               |
|             by Juergen Pfeifer <Juergen.Pfeifer@T-Online.de>                 |
|                          All Rights Reserved.                                |
|                                                                              |
| Permission to use, copy, modify, and distribute this software and its        |
| documentation for any purpose and without fee is hereby granted, provided    |
| that the above copyright notice appear in all copies and that both that      |
| copyright notice and this permission notice appear in supporting             |
| documentation, and that the name of the above listed copyright holder(s) not |
| be used in advertising or publicity pertaining to distribution of the        |
| software without specific, written prior permission.                         | 
|                                                                              |
| THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD TO  |
| THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-  |
| NESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR   |
| ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RE- |
| SULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, |
| NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH    |
| THE USE OR PERFORMANCE OF THIS SOFTWARE.                                     |
+-----------------------------------------------------------------------------*/

#include "form.priv.h"

MODULE_ID("Id: frm_def.c,v 1.4 1997/05/01 16:47:54 juergen Exp $")

/* this can't be readonly */
static FORM default_form = {
  0,                                    /* status     */
  0,                                    /* rows       */
  0,                                    /* cols       */
  0,                                    /* currow     */
  0,                                    /* curcol     */
  0,                                    /* toprow     */
  0,                                    /* begincol   */
  -1,                                   /* maxfield   */
  -1,                                   /* maxpage    */
  -1,                                   /* curpage    */
  ALL_FORM_OPTS,                        /* opts       */
  (WINDOW *)0,                          /* win        */
  (WINDOW *)0,                          /* sub        */
  (WINDOW *)0,                          /* w          */
  (FIELD **)0,                          /* field      */
  (FIELD *)0,                           /* current    */
  (_PAGE *)0,                           /* page       */
  (char *)0,                            /* usrptr     */
  NULL,			                /* forminit   */
  NULL,                                 /* formterm   */
  NULL,                                 /* fieldinit  */
  NULL                                  /* fieldterm  */
};

FORM *_nc_Default_Form = &default_form;

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static FIELD *Insert_Field_By_Position(
|                                     FIELD *new_field, 
|                                     FIELD *head )
|   
|   Description   :  Insert new_field into sorted fieldlist with head "head"
|                    and return new head of sorted fieldlist. Sorting
|                    criteria is (row,column). This is a circular list.
|
|   Return Values :  New head of sorted fieldlist
+--------------------------------------------------------------------------*/
static FIELD *Insert_Field_By_Position(FIELD *newfield, FIELD *head)
{
  FIELD *current, *newhead;
  
  assert(newfield);

  if (!head)
    { /* empty list is trivial */
      newhead = newfield->snext = newfield->sprev = newfield;
    }
  else
    {
      newhead = current = head;
      while((current->frow < newfield->frow) || 
	    ((current->frow==newfield->frow) && 
	     (current->fcol < newfield->fcol)) )
	{
	  current = current->snext;
	  if (current==head)
	    { /* We cycled through. Reset head to indicate that */
	      head = (FIELD *)0;
	      break;
	    }
	}
      /* we leave the loop with current pointing to the field after newfield*/
      newfield->snext	 = current;
      newfield->sprev	 = current->sprev;
      newfield->snext->sprev = newfield;
      newfield->sprev->snext = newfield;
      if (current==head) 
	newhead = newfield;
    }
  return(newhead);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static void Disconnect_Fields(FORM *form)
|   
|   Description   :  Break association between form and array of fields.
|
|   Return Values :  -
+--------------------------------------------------------------------------*/
static void Disconnect_Fields( FORM * form )
{
  FIELD **fields;
  
  assert(form && form->field);

  for(fields=form->field;*fields;fields++)
    {
      if (form == (*fields)->form) 
	(*fields)->form = (FORM *)0;
    }
  
  form->rows = form->cols = 0;
  form->maxfield = form->maxpage = -1;
  form->field = (FIELD **)0;
  if (form->page) 
    free(form->page);
  form->page = (_PAGE *)0;
}	

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static int Connect_Fields(FORM *form, FIELD **fields)
|   
|   Description   :  Set association between form and array of fields.
|
|   Return Values :  E_OK            - no error
|                    E_CONNECTED     - a field is already connected
|                    E_BAD_ARGUMENT  - Invalid form pointer or field array
|                    E_SYSTEM_ERROR  - not enough memory
+--------------------------------------------------------------------------*/
static int Connect_Fields(FORM  * form, FIELD ** fields)
{
  int field_cnt, j;
  int page_nr;
  int maximum_row_in_field, maximum_col_in_field;
  _PAGE *pg;
  
  assert(form);

  form->field    = fields;
  form->maxfield = 0;
  form->maxpage  = 0;

  if (!fields)
    RETURN(E_OK);
  
  page_nr = 0;
  /* store formpointer in fields and count pages */
  for(field_cnt=0;fields[field_cnt];field_cnt++)
    {
      if (fields[field_cnt]->form) 
	RETURN(E_CONNECTED);
      if ( field_cnt==0 || 
	  (fields[field_cnt]->status & _NEWPAGE)) 
	page_nr++;
      fields[field_cnt]->form = form;
    }	
  if (field_cnt==0)
    RETURN(E_BAD_ARGUMENT);
  
  /* allocate page structures */
  if ( (pg = (_PAGE *)malloc(page_nr * sizeof(_PAGE))) != (_PAGE *)0 )
    {
      form->page = pg;
    }
  else
    RETURN(E_SYSTEM_ERROR);
  
  /* Cycle through fields and calculate page boundaries as well as
     size of the form */
  for(j=0;j<field_cnt;j++)
    {
      if (j==0) 
	pg->pmin = j;
      else
	{
	  if (fields[j]->status & _NEWPAGE)
	    {
	      pg->pmax = j-1;
	      pg++;
	      pg->pmin = j;
	    }
	}
      
      maximum_row_in_field = fields[j]->frow + fields[j]->rows;
      maximum_col_in_field = fields[j]->fcol + fields[j]->cols;
      
      if (form->rows < maximum_row_in_field) 
	form->rows = maximum_row_in_field;
      if (form->cols < maximum_col_in_field) 
	form->cols = maximum_col_in_field;
    }
  
  pg->pmax       = field_cnt-1;
  form->maxfield = field_cnt;
  form->maxpage  = page_nr; 
  
  /* Sort fields on form pages */
  for(page_nr = 0;page_nr < form->maxpage; page_nr++)
    {
      FIELD *fld = (FIELD *)0;
      for(j = form->page[page_nr].pmin;j <= form->page[page_nr].pmax;j++)
	{
	  fields[j]->index = j;
	  fields[j]->page  = page_nr;
	  fld = Insert_Field_By_Position(fields[j],fld);
	}
      form->page[page_nr].smin = fld->index;
      form->page[page_nr].smax = fld->sprev->index;
    }
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  static int Associate_Fields(FORM *form, FIELD **fields)
|   
|   Description   :  Set association between form and array of fields. 
|                    If there are fields, position to first active field.
|
|   Return Values :  E_OK            - success
|                    any other       - error occured
+--------------------------------------------------------------------------*/
INLINE static int Associate_Fields(FORM  *form, FIELD **fields)
{
  int res = Connect_Fields(form,fields);
  if (res == E_OK)
    {
      if (form->maxpage>0)
	{
	  form->curpage = 0;
	  form_driver(form,FIRST_ACTIVE_MAGIC);
	}
      else
	{
	  form->curpage = -1;
	  form->current = (FIELD *)0;
	} 
    }
  return(res);
}
			    
/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  FORM *new_form( FIELD **fields )
|   
|   Description   :  Create new form with given array of fields.
|
|   Return Values :  Pointer to form. NULL if error occured.
+--------------------------------------------------------------------------*/
FORM *new_form(FIELD ** fields)
{	
  int err = E_SYSTEM_ERROR;

  FORM *form = (FORM *)malloc(sizeof(FORM));
  
  if (form)
    {
      *form = *_nc_Default_Form;
      if ((err=Associate_Fields(form,fields))!=E_OK)
	{
	  free_form(form);
	  form = (FORM *)0;
	}
    }

  if (!form)
    SET_ERROR(err);
  
  return(form);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int free_form( FORM *form )
|   
|   Description   :  Release internal memory associated with form.
|
|   Return Values :  E_OK           - no error
|                    E_BAD_ARGUMENT - invalid form pointer
|                    E_POSTED       - form is posted
+--------------------------------------------------------------------------*/
int free_form(FORM * form)
{
  if ( !form )	
    RETURN(E_BAD_ARGUMENT);

  if ( form->status & _POSTED)  
    RETURN(E_POSTED);
  
  Disconnect_Fields( form );
  if (form->page) 
    free(form->page);
  free(form);
  
  RETURN(E_OK);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int set_form_fields( FORM *form, FIELD **fields )
|   
|   Description   :  Set a new association of an array of fields to a form
|
|   Return Values :  E_OK              - no error
|                    E_BAD_ARGUMENT    - invalid form pointer
|                    E_POSTED          - form is posted
+--------------------------------------------------------------------------*/
int set_form_fields(FORM  * form, FIELD ** fields)
{
  FIELD **old;
  int res;
  
  if ( !form )	
    RETURN(E_BAD_ARGUMENT);

  if ( form->status & _POSTED )	
    RETURN(E_POSTED);
  
  old = form->field;
  Disconnect_Fields( form );
  
  if( (res = Associate_Fields( form, fields )) != E_OK )
    Connect_Fields( form, old );
  
  RETURN(res);
}
	
/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  FIELD **form_fields( const FORM *form )
|   
|   Description   :  Retrieve array of fields
|
|   Return Values :  Pointer to field array
+--------------------------------------------------------------------------*/
FIELD **form_fields(const FORM * form)
{
  return (Normalize_Form( form )->field);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int field_count( const FORM *form )
|   
|   Description   :  Retrieve number of fields
|
|   Return Values :  Number of fields, -1 if none are defined
+--------------------------------------------------------------------------*/
int field_count(const FORM * form)
{
  return (Normalize_Form( form )->maxfield);
}

/*---------------------------------------------------------------------------
|   Facility      :  libnform  
|   Function      :  int scale_form( const FORM *form, int *rows, int *cols )
|   
|   Description   :  Retrieve size of form
|
|   Return Values :  E_OK              - no error
|                    E_BAD_ARGUMENT    - invalid form pointer
|                    E_NOT_CONNECTED   - no fields connected to form
+--------------------------------------------------------------------------*/
int scale_form(const FORM * form, int * rows, int * cols)
{
  if ( !form )
    RETURN(E_BAD_ARGUMENT);

  if ( !(form->field) )
    RETURN(E_NOT_CONNECTED);
  
  if (rows) 
    *rows = form->rows;
  if (cols) 
    *cols = form->cols;
  
  RETURN(E_OK);
}

/* frm_def.c ends here */
