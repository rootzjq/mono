#include <config.h>
#include <glib.h>


#include "Directory-c-api.h"
#include "File-c-api.h"
#include "w32file.h"
#include "utils/strenc.h"

#include <winsock2.h>
#include <windows.h>
#include "mono/metadata/w32file-win32-internals.h"

void
mono_w32file_init (void)
{
}

void
mono_w32file_cleanup (void)
{
}

gunichar2
ves_icall_System_IO_MonoIO_get_VolumeSeparatorChar ()
{
	return (gunichar2) ':';	/* colon */
}

gunichar2
ves_icall_System_IO_MonoIO_get_DirectorySeparatorChar ()
{
	return (gunichar2) '\\';	/* backslash */
}

gunichar2
ves_icall_System_IO_MonoIO_get_AltDirectorySeparatorChar ()
{
	return (gunichar2) '/';	/* forward slash */
}

gunichar2
ves_icall_System_IO_MonoIO_get_PathSeparator ()
{
	return (gunichar2) ';';	/* semicolon */
}

void ves_icall_System_IO_MonoIO_DumpHandles (void)
{
	return;
}

gpointer
mono_w32file_create(const gunichar2 *name, guint32 fileaccess, guint32 sharemode, guint32 createmode, guint32 attrs)
{
	int error = 0;
	gpointer handle;
	gchar* palPath = u16to8(name);
	handle =  UnityPalOpen(palPath, (int) createmode, (int) fileaccess, (int) sharemode, attrs, &error);
	if (handle == NULL && error != 0)
		handle = INVALID_HANDLE_VALUE;

	g_free(palPath);

	return handle;
}

gboolean
mono_w32file_close (gpointer handle)
{
	int error = 0;
	return UnityPalClose(handle, &error);
}

gboolean
mono_w32file_delete (const gunichar2 *name)
{
	int error = 0;
	gchar* palPath = u16to8(name);
	gboolean result = UnityPalDeleteFile(palPath, &error);
	g_free(palPath);
	return result;
}

gboolean
mono_w32file_read(gpointer handle, gpointer buffer, guint32 numbytes, guint32 *bytesread)
{
	int error = 0;
	*bytesread =  UnityPalRead(handle, buffer, numbytes, &error);
	return (error == 0);
}

gboolean
mono_w32file_write (gpointer handle, gconstpointer buffer, guint32 numbytes, guint32 *byteswritten)
{
	int error = 0;
	*byteswritten = UnityPalWrite(handle, buffer, numbytes, &error);
	return (error == 0);
}

gboolean
mono_w32file_flush (gpointer handle)
{
	int error = 0;
	return UnityPalFlush(handle, &error);
}

gboolean
mono_w32file_truncate (gpointer handle)
{
	return SetEndOfFile (handle);
}

guint32
mono_w32file_seek (gpointer handle, gint32 movedistance, gint32 *highmovedistance, guint32 method)
{
	int error = 0;
	return UnityPalSeek(handle, movedistance, 0, &error);
}

gint
mono_w32file_get_type (gpointer handle)
{
	return UnityPalGetFileType(handle);
}

gboolean
mono_w32file_get_times (gpointer handle, FILETIME *create_time, FILETIME *access_time, FILETIME *write_time)
{
	/* Not Supported in UnityPAL */
	g_assert_not_reached();
}

gboolean
mono_w32file_set_times (gpointer handle, const FILETIME *create_time, const FILETIME *access_time, const FILETIME *write_time)
{
	int64_t creation_time = 0;
	int64_t last_access_time = 0;
	int64_t last_write_time = 0;
	int error = 0;

	/* It is possible that any one of the time values passed in could be null, need to check
	 * each one.
	 */
	if (create_time != NULL)
	{
		creation_time = (gint64)((((guint64) create_time->dwHighDateTime) << 32) + create_time->dwLowDateTime);
	}
	
	if (access_time != NULL)
	{
		last_access_time = (gint64)((((guint64)access_time->dwHighDateTime) << 32) + access_time->dwLowDateTime);
	}

	if (write_time != NULL)
	{
		last_write_time = (gint64)((((guint64) write_time->dwHighDateTime) << 32) + write_time->dwLowDateTime);
	}

	return  UnityPalSetFileTime(handle, creation_time, last_access_time, last_write_time, &error);
}

gboolean
mono_w32file_filetime_to_systemtime (const FILETIME *file_time, SYSTEMTIME *system_time)
{
	return FileTimeToSystemTime (file_time, system_time);
}

gpointer
mono_w32file_find_first (const gunichar2 *pattern, WIN32_FIND_DATA *find_data)
{
	return FindFirstFile (pattern, find_data);
}

gboolean
mono_w32file_find_next (gpointer handle, WIN32_FIND_DATA *find_data)
{
	return FindNextFile (handle, find_data);
}

gboolean
mono_w32file_find_close (gpointer handle)
{
	return FindClose (handle);
}

gboolean
mono_w32file_create_directory (const gunichar2 *name)
{
	int error = 0;
	gchar* palPath = u16to8(name);
	gboolean result = UnityPalDirectoryCreate(palPath, &error);
	g_free(palPath);
	return result;
}

gboolean
mono_w32file_remove_directory (const gunichar2 *name)
{
	int error = 0;
	gchar* palPath = u16to8(name);
	gboolean result =  UnityPalDirectoryRemove(palPath, &error);
	g_free(palPath);
	return result;
}

guint32
mono_w32file_get_attributes (const gunichar2 *name)
{
	int error = 0;
	gchar* palPath = u16to8(name);
	gboolean result =  UnityPalGetFileAttributes(u16to8(name), &error);
	g_free(palPath);
	return result;
}

gboolean
mono_w32file_get_attributes_ex (const gunichar2 *name, MonoIOStat *stat)
{
	gboolean result;
	UnityPalFileStat palStat;
	int error = 0;
	gchar* palPath = u16to8(name);

	result = UnityPalGetFileStat(palPath, &palStat, &error);

	if (result) {
		stat->attributes = palStat.attributes;
		stat->creation_time = palStat.creation_time;
		stat->last_access_time = palStat.last_access_time;
		stat->last_write_time = palStat.last_write_time;
		stat->length = palStat.length;
	}

	g_free(palPath);

	return result;
}

gboolean
mono_w32file_set_attributes (const gunichar2 *name, guint32 attrs)
{
	int error = 0;
	gchar* palPath = u16to8(name);
	
	gboolean result =  UnityPalSetFileAttributes(palPath, attrs, &error);

	g_free(palPath);
	return result;

}

guint32
mono_w32file_get_cwd(guint32 length, gunichar2 *buffer)
{
	gunichar2 *utf16_path;
	glong count;
	uintptr_t bytes;
	int error = 0;

	const char* palPath = UnityPalDirectoryGetCurrent(&error);
	utf16_path = mono_unicode_from_external(palPath, &bytes);
	count = (bytes / 2) + 1;

	memcpy(buffer, utf16_path, length);

	g_free(utf16_path);
	g_free((void *)palPath);
	return count;
}

gboolean
mono_w32file_set_cwd (const gunichar2 *path)
{
	int error = 0;
	gchar* palPath = u16to8(path);
	
	gboolean result = UnityPalDirectorySetCurrent(palPath, &error);
	
	g_free(palPath);
	return result;
}

gboolean
mono_w32file_create_pipe (gpointer *readpipe, gpointer *writepipe, guint32 size)
{
	return UnityPalCreatePipe(*readpipe, *writepipe);
}

gboolean
mono_w32file_get_disk_free_space (const gunichar2 *path_name, guint64 *free_bytes_avail, guint64 *total_number_of_bytes, guint64 *total_number_of_free_bytes)
{
	g_assert_not_reached();
	return FALSE;
}

gboolean
mono_w32file_get_volume_information (const gunichar2 *path, gunichar2 *volumename, gint volumesize, gint *outserial, gint *maxcomp, gint *fsflags, gunichar2 *fsbuffer, gint fsbuffersize)
{
	g_assert_not_reached();
	return FALSE;
}



gboolean
mono_w32file_move (gunichar2 *path, gunichar2 *dest, gint32 *error)
{
	gboolean result;

	MONO_ENTER_GC_SAFE;
	
	gchar* palPath = u16to8(path);
	gchar* palDest = u16to8(dest);
	*error = 0;
    result =  UnityPalMoveFile(palPath, palDest, error);
	g_free(palPath);
	g_free(palDest);

	MONO_EXIT_GC_SAFE;

	return result;
}

gboolean
mono_w32file_replace (gunichar2 *destinationFileName, gunichar2 *sourceFileName, gunichar2 *destinationBackupFileName, guint32 flags, gint32 *error)
{
	gboolean result;
	gchar* destPath = NULL;
	gchar* sourcePath = NULL;
	gchar* destBackupPath = NULL;

	if (destinationFileName != NULL)
	{
		destPath = u16to8(destinationFileName);
	}

	if (sourceFileName != NULL)
	{
		sourcePath = u16to8(sourceFileName);
	}

	if (destinationBackupFileName != NULL)
	{
		destBackupPath = u16to8(destinationBackupFileName);
	}

	MONO_ENTER_GC_SAFE;

	result =  UnityPalReplaceFile(sourcePath, destPath, destBackupPath, 0, error);
	*error = 0;

	MONO_EXIT_GC_SAFE;

	g_free(destPath);
	g_free(sourcePath);
	g_free(destBackupPath);

	return result;
}

gboolean
mono_w32file_copy (gunichar2 *path, gunichar2 *dest, gboolean overwrite, gint32 *error)
{
	gboolean result;

	MONO_ENTER_GC_SAFE;
	
	*error = 0;
	gchar* palPath = u16to8(path);
	gchar* palDest = u16to8(dest);
	result = UnityPalCopyFile( palPath, palDest, overwrite, error);
	g_free(palPath);
	g_free(palDest);

	MONO_EXIT_GC_SAFE;

	return result;
}

gboolean
mono_w32file_lock (gpointer handle, gint64 position, gint64 length, gint32 *error)
{

	MONO_ENTER_GC_SAFE;

	UnityPalLock(handle, position, length, error);

	MONO_EXIT_GC_SAFE;

	return (error == 0);
}

gboolean
mono_w32file_unlock (gpointer handle, gint64 position, gint64 length, gint32 *error)
{
	MONO_ENTER_GC_SAFE;

	UnityPalUnlock(handle, position, length, error);

	MONO_EXIT_GC_SAFE;

	return (error == 0);
}

HANDLE
mono_w32file_get_console_input (void)
{
	return UnityPalGetStdInput();
}

HANDLE
mono_w32file_get_console_output (void)
{
	return UnityPalGetStdOutput();
}

HANDLE
mono_w32file_get_console_error (void)
{
	return UnityPalGetStdError();
}

gint64
mono_w32file_get_file_size (gpointer handle, gint32 *error)
{
	gint64 length;

	MONO_ENTER_GC_SAFE;

	length = UnityPalGetLength(handle, error);

	MONO_EXIT_GC_SAFE;

	return length;
}

guint32
mono_w32file_get_drive_type (const gunichar2 *root_path_name)
{
	/* Not Supported in UnityPAL */
	g_assert_not_reached();
	return 0;
}

gint32
mono_w32file_get_logical_drive (guint32 len, gunichar2 *buf)
{
	/* Not Supported in UnityPAL */
	g_assert_not_reached();
	return -1;
}




