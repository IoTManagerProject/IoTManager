# -*- coding: utf-8 -*-
"""
Тесты каталога данных ФС (data_dir) панели magicIoTm.

Проверяется механика сбоя `pio run -t uploadfs` с сообщениями
«warning: can't read source directory» и «[.pio/build/<env>/littlefs.bin] Error 1»:
PlatformIO передаёт значение `[platformio] data_dir` из platformio.ini в
`mklittlefs -c` (как `$PROJECT_DATA_DIR`), поэтому устаревший путь (после переноса
проекта или переименования категории) валит сборку образа LittleFS.

Запуск (из каталога tools/magicIoTm):

    python -m unittest discover -s tests -v

Если установлен pytest — те же тесты запускаются и им:

    python -m pytest tests/test_data_dir.py -v

Сеть, PlatformIO и подключённое железо не нужны: файловые проверки идут во
временных каталогах, API-тесты используют Flask test-client, а запуск прошивки
подменяется заглушкой (реального `pio` и COM-порта нет).
"""

import os
import shutil
import sys
import tempfile
import unittest

# Каталог tools/magicIoTm — корень пакетов magicIoTm (utils, core, routes, state)
MAGICIOTM_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if MAGICIOTM_DIR not in sys.path:
    sys.path.insert(0, MAGICIOTM_DIR)

from utils import projects  # noqa: E402

# Имя временной категории проектов для API-тестов.
# Начинается с точки — list_projects() такие записи пропускает, поэтому даже при
# аварийном завершении теста папка не попадёт в UI панели.
SELFTEST_CATEGORY = '.selftest_data_dir'

# Каталог для временных проектов файловых тестов. Лежит внутри projects/ (gitignored)
# и на том же диске, что и репозиторий, — иначе os.path.relpath не сможет посчитать
# путь для platformio.ini. Имя с точкой → list_projects() его пропускает.
SELFTEST_TMP_DIR = os.path.join(projects.PROJECTS_DIR, '.selftest_tmp')


def _temp_project_dir(prefix):
    """Создаёт временный каталог под тесты на диске репозитория."""
    os.makedirs(SELFTEST_TMP_DIR, exist_ok=True)
    return tempfile.mkdtemp(prefix=prefix, dir=SELFTEST_TMP_DIR)


class IniHelpersTest(unittest.TestCase):
    """Чтение и запись строки data_dir в platformio.ini."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix='iotm_ini_')
        self.ini = os.path.join(self.tmp, 'platformio.ini')

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _write_ini(self, text, encoding='utf-8'):
        with open(self.ini, 'w', encoding=encoding, newline='') as f:
            f.write(text)

    def test_read_returns_value_from_platformio_section(self):
        self._write_ini('[platformio]\n'
                        'default_envs = esp32s2_4mb\n'
                        'data_dir = tools/magicIoTm/projects/Платы/esp32s2mini/data_svelte\n')
        self.assertEqual(
            projects.read_ini_data_dir(self.ini),
            'tools/magicIoTm/projects/Платы/esp32s2mini/data_svelte')

    def test_read_ignores_data_dir_from_other_sections(self):
        self._write_ini('[env:esp32s2_4mb]\ndata_dir = неверный\n')
        self.assertIsNone(projects.read_ini_data_dir(self.ini))

    def test_read_missing_file_returns_none(self):
        self.assertIsNone(projects.read_ini_data_dir(os.path.join(self.tmp, 'нет-файла.ini')))

    def test_read_falls_back_to_cp1251(self):
        # старые ini писались в cp1251 — PrepareProject.py читает их так же
        self._write_ini('[platformio]\n# путь к данным устройства\n'
                        'data_dir = tools/magicIoTm/projects/Платы/тест/data_svelte\n',
                        encoding='cp1251')
        self.assertEqual(projects.read_ini_data_dir(self.ini),
                         'tools/magicIoTm/projects/Платы/тест/data_svelte')

    def test_set_replaces_only_data_dir_and_keeps_comments(self):
        self._write_ini('; комментарий сверху файла\n'
                        '[platformio]\n'
                        '# data_dir — каталог данных ФС\n'
                        'default_envs = esp32s2_4mb\n'
                        'data_dir = tools/magicIoTm/projects/Test/x/data_svelte\n'
                        '\n'
                        '[env]\n'
                        'extra_scripts = pre:tools/prebuildscript.py\n')
        ok, msg = projects.set_ini_data_dir(
            self.ini, 'tools/magicIoTm/projects/Платы/x/data_svelte')
        self.assertTrue(ok, msg)

        with open(self.ini, encoding='utf-8') as f:
            text = f.read()
        self.assertIn('; комментарий сверху файла', text)
        self.assertIn('# data_dir — каталог данных ФС', text)
        self.assertIn('[env]\nextra_scripts = pre:tools/prebuildscript.py', text)
        self.assertIn('default_envs = esp32s2_4mb', text)
        self.assertIn('tools/magicIoTm/projects/Платы/x/data_svelte', text)
        self.assertNotIn('projects/Test/x', text)

    def test_set_inserts_option_when_missing(self):
        self._write_ini('[platformio]\ndefault_envs = esp32s2_4mb\n\n[env]\nframework = arduino\n')
        ok, msg = projects.set_ini_data_dir(self.ini, 'data_svelte')
        self.assertTrue(ok, msg)
        self.assertEqual(projects.read_ini_data_dir(self.ini), 'data_svelte')
        with open(self.ini, encoding='utf-8') as f:
            lines = f.read().splitlines()
        # ключ добавлен сразу после заголовка секции [platformio]
        self.assertEqual(lines[1], 'data_dir = data_svelte')

    def test_set_keeps_crlf_line_endings(self):
        self._write_ini('[platformio]\r\ndata_dir = old\r\n')
        ok, msg = projects.set_ini_data_dir(self.ini, 'new')
        self.assertTrue(ok, msg)
        with open(self.ini, 'rb') as f:
            self.assertEqual(f.read(), b'[platformio]\r\ndata_dir = new\r\n')

    def test_set_fails_without_platformio_section(self):
        self._write_ini('[env]\nframework = arduino\n')
        ok, msg = projects.set_ini_data_dir(self.ini, 'data_svelte')
        self.assertFalse(ok)
        self.assertIn('[platformio]', msg)

    def test_set_fails_on_missing_file(self):
        ok, msg = projects.set_ini_data_dir(os.path.join(self.tmp, 'нет-файла.ini'), 'x')
        self.assertFalse(ok)
        self.assertIn('не найден', msg)


class FixDataDirTest(unittest.TestCase):
    """fix_data_dir/sync_data_dir: путь записывается относительно корня репозитория."""

    def setUp(self):
        # каталог на диске репозитория: data_dir считается относительным путём
        self.tmp = _temp_project_dir('fix_')
        self.proj = os.path.join(self.tmp, 'proj')
        os.makedirs(os.path.join(self.proj, projects.DATA_DIR_NAME))
        self.ini = os.path.join(self.proj, projects.PLATFORMIO_INI_FILENAME)
        with open(self.ini, 'w', encoding='utf-8', newline='') as f:
            f.write('[platformio]\n'
                    'default_envs = esp32s2_4mb\n'
                    'data_dir = tools/magicIoTm/projects/Test/proj/data_svelte\n')

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    @classmethod
    def tearDownClass(cls):
        try:
            os.rmdir(SELFTEST_TMP_DIR)  # удалится, только если каталог пуст
        except OSError:
            pass

    def test_fix_writes_path_relative_to_repo_root(self):
        ok, value = projects.fix_data_dir(self.proj, cwd=projects.REPO_ROOT)
        self.assertTrue(ok, value)
        expected = os.path.relpath(os.path.join(self.proj, projects.DATA_DIR_NAME),
                                   projects.REPO_ROOT).replace(os.sep, '/')
        self.assertEqual(value, expected)
        self.assertEqual(projects.read_ini_data_dir(self.ini), expected)

    def test_fix_without_data_svelte_fails_and_keeps_ini(self):
        shutil.rmtree(os.path.join(self.proj, projects.DATA_DIR_NAME))
        ok, msg = projects.fix_data_dir(self.proj)
        self.assertFalse(ok)
        self.assertIn('Каталог данных не найден', msg)
        # ini не должен быть изменён
        self.assertEqual(projects.read_ini_data_dir(self.ini),
                         'tools/magicIoTm/projects/Test/proj/data_svelte')

    def test_fix_missing_project_dir_fails(self):
        ok, msg = projects.fix_data_dir(os.path.join(self.tmp, 'нет-проекта'))
        self.assertFalse(ok)
        self.assertIn('не найден', msg)

    def test_sync_is_noop_without_ini(self):
        os.remove(self.ini)
        projects.sync_data_dir(self.proj)  # исключений быть не должно
        self.assertFalse(os.path.exists(self.ini))

    def test_sync_is_noop_without_data_svelte(self):
        shutil.rmtree(os.path.join(self.proj, projects.DATA_DIR_NAME))
        projects.sync_data_dir(self.proj)
        self.assertEqual(projects.read_ini_data_dir(self.ini),
                         'tools/magicIoTm/projects/Test/proj/data_svelte')

    def test_sync_fixes_stale_path(self):
        projects.sync_data_dir(self.proj)
        value = projects.read_ini_data_dir(self.ini)
        self.assertTrue(value.endswith('proj/data_svelte'))
        self.assertNotIn('Test', value)

    @unittest.skipIf(
        os.path.splitdrive(tempfile.gettempdir())[0].lower()
        == os.path.splitdrive(projects.REPO_ROOT)[0].lower(),
        'для проверки запасного варианта нужен диск, отличный от диска репозитория')
    def test_fix_falls_back_to_absolute_path_on_other_drive(self):
        # os.path.relpath невозможен между дисками — путь должен остаться абсолютным
        ok, value = projects.fix_data_dir(self.proj, cwd=tempfile.gettempdir())
        self.assertTrue(ok, value)
        self.assertEqual(value, os.path.abspath(
            os.path.join(self.proj, projects.DATA_DIR_NAME)).replace(os.sep, '/'))
        self.assertEqual(projects.read_ini_data_dir(self.ini), value)


class DataDirReportTest(unittest.TestCase):
    """data_dir_report/data_dir_error_text: причины для предпроверки прошивки."""

    def setUp(self):
        self.tmp = tempfile.mkdtemp(prefix='iotm_report_')
        self.proj = os.path.join(self.tmp, 'proj')
        self.data_dir = os.path.join(self.proj, projects.DATA_DIR_NAME)
        os.makedirs(self.data_dir)
        self.ini = os.path.join(self.proj, projects.PLATFORMIO_INI_FILENAME)

    def tearDown(self):
        shutil.rmtree(self.tmp, ignore_errors=True)

    def _write_ini(self, value):
        with open(self.ini, 'w', encoding='utf-8', newline='') as f:
            f.write('[platformio]\ndefault_envs = esp32s2_4mb\ndata_dir = {}\n'.format(value))

    def test_ok(self):
        self._write_ini(projects.DATA_DIR_NAME)
        rep = projects.data_dir_report(self.ini, self.data_dir, self.proj)
        self.assertTrue(rep['ok'])
        self.assertEqual(rep['reason'], '')
        self.assertEqual(projects.data_dir_error_text(rep), '')

    def test_missing_when_directory_does_not_exist(self):
        self._write_ini('tools/magicIoTm/projects/Test/old/data_svelte')
        rep = projects.data_dir_report(self.ini, self.data_dir, self.proj)
        self.assertFalse(rep['ok'])
        self.assertEqual(rep['reason'], 'missing')
        self.assertIn('Test', rep['ini_value'])
        self.assertEqual(rep['expected'], os.path.abspath(self.data_dir))

        text = projects.data_dir_error_text(rep)
        self.assertIn('не найден', text)
        self.assertIn(rep['expected'], text)

    def test_mismatch_when_other_existing_directory(self):
        # путь существует, но это не data_svelte проекта (например, data_full)
        os.makedirs(os.path.join(self.proj, 'data_full'))
        self._write_ini('data_full')
        rep = projects.data_dir_report(self.ini, self.data_dir, self.proj)
        self.assertFalse(rep['ok'])
        self.assertEqual(rep['reason'], 'mismatch')
        self.assertIn('не совпадает', projects.data_dir_error_text(rep))

    def test_no_option(self):
        with open(self.ini, 'w', encoding='utf-8', newline='') as f:
            f.write('[platformio]\ndefault_envs = esp32s2_4mb\n')
        rep = projects.data_dir_report(self.ini, self.data_dir, self.proj)
        self.assertEqual(rep['reason'], 'no_option')
        self.assertIn('не задан', projects.data_dir_error_text(rep))

    def test_no_ini(self):
        rep = projects.data_dir_report(os.path.join(self.proj, 'нет.ini'), self.data_dir, self.proj)
        self.assertFalse(rep['ok'])
        self.assertEqual(rep['reason'], 'no_ini')

    def test_absolute_path_in_ini_is_supported(self):
        self._write_ini(self.data_dir.replace(os.sep, '/'))
        rep = projects.data_dir_report(self.ini, self.data_dir, self.proj)
        self.assertTrue(rep['ok'], rep)

    def test_core_builder_wrapper_uses_project_profile_dir(self):
        from core import builder

        self._write_ini(projects.DATA_DIR_NAME)
        cfg = {
            'ini': self.ini,
            'profile': os.path.join(self.proj, projects.CONFIG_FILENAME),
            'cwd': self.proj,
        }
        rep = builder.data_dir_report(cfg)
        self.assertTrue(rep['ok'], rep)
        self.assertEqual(rep['expected'], os.path.abspath(self.data_dir))
        self.assertIs(builder.data_dir_error_text, projects.data_dir_error_text)


class RepairDataDirApiTest(unittest.TestCase):
    """API: предпроверка в POST /api/upload/start и POST /api/projects/repair-data-dir.

    Проект создаётся во временной категории внутри tools/magicIoTm/projects,
    потому что маршруты вычисляют путь проекта из projects.PROJECTS_DIR.
    Запуск прошивки подменяется заглушкой: реальный pio/COM-порт не используются.
    """

    @classmethod
    def setUpClass(cls):
        import app as application
        import routes.upload as upload_routes
        import state.globals as globals_

        cls.application = application
        cls.upload_routes = upload_routes
        cls.globals = globals_

    def setUp(self):
        self.client = self.application.app.test_client()

        self.cat_dir = os.path.join(projects.PROJECTS_DIR, SELFTEST_CATEGORY)
        shutil.rmtree(self.cat_dir, ignore_errors=True)
        self.proj = os.path.join(self.cat_dir, 'projA')
        os.makedirs(os.path.join(self.proj, projects.DATA_DIR_NAME))

        # myProfile.json проекта (минимально достаточный для open/прошивки)
        with open(os.path.join(self.proj, projects.CONFIG_FILENAME), 'w', encoding='utf-8') as f:
            f.write('{"iotmSettings": {"name": "selftestA"}, "modules": {},'
                    ' "projectProp": {"platformio": {"default_envs": "esp32s2_4mb"}}}')

        # устаревший data_dir — как после переименования категории Test -> Платы
        self.ini = os.path.join(self.proj, projects.PLATFORMIO_INI_FILENAME)
        with open(self.ini, 'w', encoding='utf-8', newline='') as f:
            f.write('[platformio]\n'
                    'default_envs = esp32s2_4mb\n'
                    'data_dir = tools/magicIoTm/projects/Test/projA/data_svelte\n')

        self._originals = {
            'flash_start': self.upload_routes.flash_start,
            'has_built_firmware': self.upload_routes._has_built_firmware,
            'ensure_installed': self.upload_routes.ensure_installed,
            'detect_device': self.upload_routes.detect_device,
            'current_project': self.globals.current_project,
            'current_config': self.globals.current_config,
        }
        self.flashed = []
        self.upload_routes._has_built_firmware = lambda cfg: True
        self.upload_routes.ensure_installed = lambda: None
        self.upload_routes.detect_device = lambda port: None
        self.upload_routes.flash_start = lambda cfg: self.flashed.append(cfg)
        self.globals.current_project = None
        self.globals.current_config = {}

    def tearDown(self):
        self.upload_routes.flash_start = self._originals['flash_start']
        self.upload_routes._has_built_firmware = self._originals['has_built_firmware']
        self.upload_routes.ensure_installed = self._originals['ensure_installed']
        self.upload_routes.detect_device = self._originals['detect_device']
        self.globals.current_project = self._originals['current_project']
        self.globals.current_config = self._originals['current_config']
        shutil.rmtree(self.cat_dir, ignore_errors=True)

    def _open_project(self):
        r = self.client.post('/api/projects/{}/projA/open'.format(SELFTEST_CATEGORY))
        self.assertEqual(r.status_code, 200, r.get_data(as_text=True))
        self.assertTrue(r.get_json()['success'])

    def test_upload_start_blocks_on_stale_data_dir(self):
        self._open_project()
        r = self.client.post('/api/upload/start', json={'mode': 'fs', 'port': 'COM99'})
        self.assertEqual(r.status_code, 409)
        d = r.get_json()
        self.assertFalse(d['success'])
        self.assertEqual(d['code'], 'data_dir')
        self.assertIn('Test', d['ini_value'])
        self.assertTrue(d['expected'].endswith(
            os.path.join(SELFTEST_CATEGORY, 'projA', projects.DATA_DIR_NAME)), d['expected'])
        self.assertEqual(d['project'], {'category': SELFTEST_CATEGORY, 'name': 'projA'})
        # прошивка не должна запускаться
        self.assertEqual(self.flashed, [])

    def test_repair_endpoint_rewrites_data_dir(self):
        self._open_project()
        r = self.client.post('/api/projects/repair-data-dir',
                             json={'category': SELFTEST_CATEGORY, 'name': 'projA'})
        self.assertEqual(r.status_code, 200, r.get_data(as_text=True))
        d = r.get_json()
        self.assertTrue(d['success'])

        expected = os.path.relpath(os.path.join(self.proj, projects.DATA_DIR_NAME),
                                   projects.REPO_ROOT).replace(os.sep, '/')
        self.assertEqual(d['data_dir'], expected)
        self.assertEqual(projects.read_ini_data_dir(self.ini), expected)

    def test_upload_start_passes_after_repair(self):
        self._open_project()
        self.client.post('/api/projects/repair-data-dir',
                         json={'category': SELFTEST_CATEGORY, 'name': 'projA'})
        r = self.client.post('/api/upload/start', json={'mode': 'full', 'port': 'COM99'})
        self.assertEqual(r.status_code, 200, r.get_data(as_text=True))
        self.assertTrue(r.get_json()['success'])
        # предпроверка пройдена — дело дошло до запуска прошивки
        self.assertEqual(len(self.flashed), 1)
        self.assertEqual(self.flashed[0]['mode'], 'full')

    def test_repair_requires_project_fields(self):
        r = self.client.post('/api/projects/repair-data-dir', json={})
        self.assertEqual(r.status_code, 400)
        self.assertFalse(r.get_json()['success'])

    def test_repair_unknown_project(self):
        r = self.client.post('/api/projects/repair-data-dir',
                             json={'category': SELFTEST_CATEGORY, 'name': 'нет-такого'})
        self.assertEqual(r.status_code, 404)


if __name__ == '__main__':
    unittest.main(verbosity=2)
