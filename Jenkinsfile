pipeline {
  agent any
  stages {
    stage('Init') {
      steps {
        echo "Testing..."
      }
    }
    stage('Build') {
      steps {
        echo "Building..."
        sh 'hostname'
        sh '/usr/bin/git --version'
        sh '/usr/bin/gcc --version'
        sh '/usr/bin/g++ --version'
      }
    }
    stage('Deploy') {
      steps {
        echo "Code deployed."
      }
    }
  }
}
