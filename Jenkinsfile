pipeline {
  agent any
  stages {
    stage('Init') {
      steps {
        echo "Testing..."
      }
    }
    stage('Build') {
      parallel {
        steps {
          echo "Building..."
        }
        steps {
          sh 'hostname'
        }
        steps {
          sh '/usr/bin/git --version'
        }
        steps {
          sh '/usr/bin/gcc --version'
        }
        steps {
          sh '/usr/bin/g++ --version'
        }
      }
    }
    stage('Deploy') {
      steps {
        echo "Code deployed."
      }
    }
  }
}
