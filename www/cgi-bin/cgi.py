import subprocess

def run_script_with_timeout(script_path, timeout):
    try:
        # Lancer le script enfant
        result = subprocess.run(['python3', script_path], timeout=timeout)
        print(f"Script exited with return code: {result.returncode}")
    except subprocess.TimeoutExpired:
        print(f"Script execution exceeded {timeout} seconds and was terminated.")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    # Chemin du script à exécuter
    script_to_run = "test.py"  # Change avec le nom de ton script
    timeout_duration = 3  # Timeout en secondes

    # Exécuter le script avec un timeout
    run_script_with_timeout(script_to_run, timeout_duration)
