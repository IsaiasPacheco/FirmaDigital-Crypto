#include <gtk/gtk.h>
#include <string.h>
#include "cifra.h"

GObject *window;
GtkTextBuffer * bufferLog;
GtkBuilder *builder;
GObject *btnCargarLlave;
GObject *btnCargarArchivo;
GObject *ComboBox;
GObject *textViewContenido;
GObject *txtNombreArchivo;
GObject *txtNombreLlave;
GObject *txtPass;
GObject *btnAceptar;
GtkWidget *msjDialog;

char * llavePath = NULL;
char * archivoPath = NULL;

static void mostrarMsj( char * titulo, char * msj){
  msjDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            titulo);
  gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(msjDialog),
            msj);  
  gtk_dialog_run(GTK_DIALOG(msjDialog));

  gtk_widget_destroy(msjDialog);  
}

static void resetGUI(){
  llavePath = NULL;
  archivoPath = NULL;
  gtk_entry_set_text ((GtkEntry *)txtPass, "");
  gtk_label_set_text ((GtkLabel *)txtNombreArchivo,"El archivo no esta cargado");
  gtk_label_set_text ((GtkLabel *)txtNombreLlave,"La llave no esta cargada");
}

static void cargarLlave( GtkWidget *widget, gpointer   data){
  g_print ("Cargando Llave...\n");
  GtkWidget *dialog;
  GtkWindow * parent = (GtkWindow *)  window;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;

  dialog = gtk_file_chooser_dialog_new ("Open File", parent, action, "Cancelar",
                                      GTK_RESPONSE_CANCEL,
                                      "Abrir",
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);

res = gtk_dialog_run (GTK_DIALOG (dialog));
if (res == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    filename = gtk_file_chooser_get_filename (chooser);
    //open_file (filename);
    gtk_text_buffer_insert_at_cursor (bufferLog,"Se selecciono la llave: ",-1);
    gtk_text_buffer_insert_at_cursor (bufferLog, filename,-1);
    gtk_text_buffer_insert_at_cursor (bufferLog,"\n",-1);

    if( gtk_combo_box_get_active ((GtkComboBox *)ComboBox) == 0 ){
      gtk_text_buffer_insert_at_cursor (bufferLog,"Llave privada detectada, ingrese la contraseña PEM\n",-1);
    }else{
      gtk_text_buffer_insert_at_cursor (bufferLog,"Llave public detectada, no es necesaria la contraseña PEM\n",-1);
    }

    gtk_label_set_text((GtkLabel *)txtNombreLlave, "Llave cargada");

    llavePath = (char *)malloc(sizeof(char)*strlen(filename));
    strcpy(llavePath, filename);
    g_free (filename);
  }

gtk_widget_destroy (dialog);

}

static void cargarArchivo( GtkWidget *widget, gpointer   data){
  g_print ("Cargando Archivo...\n");
  GtkWidget *dialog;
  GtkWindow * parent = (GtkWindow *)  window;
  GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
  gint res;

  dialog = gtk_file_chooser_dialog_new ("Open File", parent, action, "Cancelar",
                                      GTK_RESPONSE_CANCEL,
                                      "Abrir",
                                      GTK_RESPONSE_ACCEPT,
                                      NULL);

res = gtk_dialog_run (GTK_DIALOG (dialog));
if (res == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (dialog);
    filename = gtk_file_chooser_get_filename (chooser);
    //open_file (filename);
    gtk_text_buffer_insert_at_cursor (bufferLog,"Se selecciono el archivo: ",-1);
    gtk_text_buffer_insert_at_cursor (bufferLog, filename,-1);
    gtk_text_buffer_insert_at_cursor (bufferLog,"\n",-1);

    gtk_label_set_text((GtkLabel *)txtNombreArchivo, "Archivo cargado");

    archivoPath = (char *)malloc(sizeof(char)*strlen(filename));
    strcpy(archivoPath, filename);
    g_free (filename);
  }

gtk_widget_destroy (dialog);

}

static void accionBotonAceptar(GtkWidget *widget, gpointer   data){
  //Comprobar que esta cargado el nombre del archivo y de la llave
  //Comrpobar la pass
  if( archivoPath != NULL || llavePath != NULL ){
    if( gtk_combo_box_get_active ((GtkComboBox *)ComboBox) == 0 ){
      //Firmar
      const char * psw = gtk_entry_get_text((GtkEntry *)txtPass);
      if( strlen(psw) == 0 ){
        mostrarMsj("Error", "Ingrese la contraseña");
      }else{
        if( !(firmarArchivo(archivoPath, "firma.sha", llavePath, psw)) ){
            mostrarMsj("Error", "Error al firmar el archivo");
        }else{
            mostrarMsj("Información", "Archivo firmado correctamente");
            resetGUI();
        }
      }
    }else{
      //Comrpobar
      int res = comprobarArchivo(archivoPath, llavePath);
      if(res == 0 ){
        mostrarMsj("Error", "Error al comprobar el archivo");
      }else if( res == 1){
        mostrarMsj("Información", "No hubo alteración en el archivo");
      }else if( res == 2){
        mostrarMsj("Cuidado", "Alteración en el archivo");
      }
    }
  }else{
    msjDialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        "Error");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(msjDialog),
        "El Archivo o la Llave no estan cargados");  
    gtk_dialog_run(GTK_DIALOG(msjDialog));

    gtk_widget_destroy(msjDialog);
  }


}

static void cambioDeEstadoCombo(GtkWidget *widget, gpointer   data){
  resetGUI();
  //Limpiar la pass, los archivos cargas y los txt de archivo y llave
}

int main (int   argc, char *argv[])
{
  GError *error = NULL;

  gtk_init (&argc, &argv);

  /* Construct a GtkBuilder instance and load our UI description */
  builder = gtk_builder_new ();
  if (gtk_builder_add_from_file (builder, "gui.glade", &error) == 0)
    {
      g_printerr ("Error loading file: %s\n", error->message);
      g_clear_error (&error);
      return 1;
    }

  /*Mapear el archivo de interfaz */
  window = gtk_builder_get_object (builder, "window");
  g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

  btnCargarLlave = gtk_builder_get_object (builder, "btnLlave");
  g_signal_connect (btnCargarLlave, "clicked", G_CALLBACK (cargarLlave), NULL);

  btnCargarArchivo = gtk_builder_get_object (builder, "btnArchivo");
  g_signal_connect (btnCargarArchivo, "clicked", G_CALLBACK (cargarArchivo), NULL);

  ComboBox = gtk_builder_get_object (builder, "comboBoxOpciones");
  g_signal_connect (ComboBox, "changed", G_CALLBACK (cambioDeEstadoCombo), NULL);

  textViewContenido = gtk_builder_get_object (builder, "txtContenido");
  bufferLog = gtk_text_buffer_new (NULL);
  gtk_text_view_set_buffer ((GtkTextView *) textViewContenido,bufferLog);
  
  //GTKLabel para el nombre del archivo y llave
  txtNombreArchivo = gtk_builder_get_object (builder, "txtNombreArchivo");
  txtNombreLlave = gtk_builder_get_object (builder, "txtNombreArchivoLlave");

  //GtkText para la pass 
  txtPass = gtk_builder_get_object (builder, "txtPEMPassword");

  //Boton aceptar
  btnAceptar = gtk_builder_get_object (builder, "btnAceptar");
  g_signal_connect (btnAceptar, "clicked", G_CALLBACK (accionBotonAceptar), NULL);

  //Txt pass modo password
  gtk_entry_set_visibility ( (GtkEntry *)txtPass, FALSE);

  //Establecer firma como elemento activo del ComboBox
  gtk_combo_box_set_active ( (GtkComboBox *) ComboBox, 0);
  //gtk_text_buffer_insert_at_cursor (bufferLog,"hola",-1);
  gtk_main ();

  return 0;
}